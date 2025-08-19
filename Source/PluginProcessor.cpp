#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompressorPluginAudioProcessor::CompressorPluginAudioProcessor()
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
 #if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
 #endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
)
, apvts(*this, nullptr, "PARAMS", createParameterLayout())
{
    // Oversampling disabled to prevent crashes
    // oversampling = nullptr;
    // oversamplingEnabled = false;
    
    // Initialize upwards compressor state variables to prevent audio pops
    upwardsEnv = 1.0e-12f; // Small non-zero value to prevent division by zero
    upwardsSmoothGain = 1.0f;
    upwardsAttackCoeff = 0.0f;
    upwardsReleaseCoeff = 0.0f;
    currentUpwardsGaindB.store(0.0f);
    upwardsInitialRamp = true; // Start in initial ramp mode
    upwardsStartupDelay = 0; // Reset startup delay
    audioIsActive = false; // Start with audio inactive
    audioInactiveCounter = 0; // Reset inactive counter
}

CompressorPluginAudioProcessor::~CompressorPluginAudioProcessor() = default;

//==============================================================================
bool CompressorPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void CompressorPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int numIn = juce::jmax (1, getTotalNumInputChannels());
    wetBuffer.setSize (numIn, samplesPerBlock);
    scBuffer.setSize (1, samplesPerBlock);

    // Initialize the sidechain EQ filter
    scEQCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1500.0, 0.7071f, 1.0f);
    scEQ.prepare({sampleRate, (juce::uint32)samplesPerBlock, 1});
    scEQ.coefficients = scEQCoeffs;

    // Initialize envelope followers to prevent pops when audio starts
    env = 1.0e-12f;
    upwardsEnv = 1.0e-12f;
    smoothGain = 1.0f;
    upwardsSmoothGain = 1.0f;
    upwardsInitialRamp = true; // Reset initial ramp mode
    upwardsStartupDelay = 0; // Reset startup delay
    audioIsActive = false; // Reset audio active state
    audioInactiveCounter = 0; // Reset inactive counter

    updateTimeConstants();
    updateSidechainEQ();
}

void CompressorPluginAudioProcessor::releaseResources() 
{
    // Oversampling disabled to prevent crashes
    // oversampling = nullptr;
}

void CompressorPluginAudioProcessor::updateTimeConstants()
{
    const float attackMs  = apvts.getRawParameterValue ("ATTACK")->load();
    const float releaseMs = apvts.getRawParameterValue ("RELEASE")->load();
    const double sr = juce::jmax (1.0, getSampleRate());
    // tiny +1 inside to avoid zero divisions in pathological cases
    attackCoeff  = std::exp (-1.0f / ((float) (attackMs  * 0.001 * sr) + 1.0f));
    releaseCoeff = std::exp (-1.0f / ((float) (releaseMs * 0.001 * sr) + 1.0f));

    // Upwards compressor time constants
    const float upwardsAttackMs  = apvts.getRawParameterValue ("UPWARDS_ATTACK")->load();
    const float upwardsReleaseMs = apvts.getRawParameterValue ("UPWARDS_RELEASE")->load();
    upwardsAttackCoeff  = std::exp (-1.0f / ((float) (upwardsAttackMs  * 0.001 * sr) + 1.0f));
    upwardsReleaseCoeff = std::exp (-1.0f / ((float) (upwardsReleaseMs * 0.001 * sr) + 1.0f));
}



void CompressorPluginAudioProcessor::updateSidechainEQ()
{
    // Sidechain EQ modes: boost or cut at 1.5 kHz for different compression focuses
    const double sr = juce::jmax (1.0, getSampleRate());
    const double freq = 1500.0;
    const float  q    = 0.7071f; // wide, musical Q
    
    if (vocalModeEnabled)
    {
        // Vocal mode: threshold-coupled peak gain, up to +5 dB as threshold lowers
        const float thresholdMin = -60.0f;
        const float thresholdMax = 0.0f;
        const float thr = apvts.getRawParameterValue ("THRESHOLD")->load();
        const float tNorm = juce::jlimit (0.0f, 1.0f, (thresholdMax - thr) / (thresholdMax - thresholdMin));
        const float peakDb = tNorm * 5.0f; // 0 .. +5 dB
        const float peakGain = juce::Decibels::decibelsToGain(peakDb);
        scEQCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr, freq, q, peakGain);
    }
    else if (drumbusModeEnabled)
    {
        // Drumbus mode: threshold-coupled peak cut, up to -5 dB as threshold lowers
        const float thresholdMin = -60.0f;
        const float thresholdMax = 0.0f;
        const float thr = apvts.getRawParameterValue ("THRESHOLD")->load();
        const float tNorm = juce::jlimit (0.0f, 1.0f, (thresholdMax - thr) / (thresholdMax - thresholdMin));
        const float peakDb = tNorm * -5.0f; // 0 .. -5 dB
        const float peakGain = juce::Decibels::decibelsToGain(peakDb);
        scEQCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr, freq, q, peakGain);
    }
    else
    {
        // Normal mode: no sidechain EQ boost
        scEQCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr, freq, q, 1.0f);
    }
    
    scEQ.coefficients = scEQCoeffs;
}

float CompressorPluginAudioProcessor::computeGain (float scSample) noexcept
{
    // RMS detector with optimized smoothing to reduce aliasing
    const float x2 = scSample * scSample;
    env = x2 + (env - x2) * 0.99f; // Faster response, less smoothing
    
    // Ensure envelope doesn't get stuck at zero
    if (env < 1.0e-12f) env = 1.0e-12f;
    
    const float rms = std::sqrt (env);
    const float levelDb = juce::Decibels::gainToDecibels (rms);

    const float thr   = apvts.getRawParameterValue ("THRESHOLD")->load();
    const float ratio = apvts.getRawParameterValue ("RATIO")->load();
    const float knee  = apvts.getRawParameterValue ("KNEE")->load();

    const float over = levelDb - thr;
    float grDb = 0.0f;

    if (knee > 0.0f)
    {
        const float halfKnee = 0.5f * knee;
        if (over > -halfKnee && over < halfKnee)
        {
            const float x = (over + halfKnee) / juce::jmax (1.0e-6f, knee); // 0..1
            const float soft = x * x * (3.0f - 2.0f * x);                    // smoothstep
            grDb = soft * (over - over / juce::jmax (1.0f, ratio));
        }
        else if (over >= halfKnee)
        {
            grDb = (over - over / juce::jmax (1.0f, ratio));
        }
    }
    else if (over > 0.0f)
    {
        grDb = (over - over / juce::jmax (1.0f, ratio));
    }

    const float target = juce::Decibels::decibelsToGain (-grDb);
    if (target < smoothGain) smoothGain = smoothGain * attackCoeff  + target * (1.0f - attackCoeff);
    else                     smoothGain = smoothGain * releaseCoeff + target * (1.0f - releaseCoeff);

    currentGRdB.store (juce::jlimit (0.0f, 60.0f, -juce::Decibels::gainToDecibels (smoothGain + 1.0e-9f)));
    return smoothGain;
}

float CompressorPluginAudioProcessor::computeUpwardsGain (float scSample) noexcept
{
    // RMS detector with much slower initial response to prevent pops
    const float x2 = scSample * scSample;
    
    // Use a very slow initial ramp to prevent sudden jumps when audio starts
    if (upwardsInitialRamp)
    {
        upwardsEnv = x2 * 0.001f + upwardsEnv * 0.999f; // Very slow initial ramp
        if (upwardsEnv > 1.0e-6f) upwardsInitialRamp = false; // Switch to normal mode once we have some signal
    }
    else
    {
        upwardsEnv = x2 + (upwardsEnv - x2) * 0.99f; // Normal response
    }
    
    // Ensure envelope doesn't get stuck at zero
    if (upwardsEnv < 1.0e-12f) upwardsEnv = 1.0e-12f;
    
    const float rms = std::sqrt (upwardsEnv);
    const float levelDb = juce::Decibels::gainToDecibels (rms);

    const float thr   = apvts.getRawParameterValue ("UPWARDS_THRESHOLD")->load();
    const float ratio = apvts.getRawParameterValue ("UPWARDS_RATIO")->load();
    const float knee  = apvts.getRawParameterValue ("UPWARDS_KNEE")->load();

    const float under = thr - levelDb; // For upwards compression, we look at how much we're UNDER the threshold
    float gainDb = 0.0f;

    if (knee > 0.0f)
    {
        const float halfKnee = 0.5f * knee;
        if (under > -halfKnee && under < halfKnee)
        {
            const float x = (under + halfKnee) / juce::jmax (1.0e-6f, knee); // 0..1
            const float soft = x * x * (3.0f - 2.0f * x);                    // smoothstep
            gainDb = soft * (under - under / juce::jmax (1.0f, ratio));
        }
        else if (under >= halfKnee)
        {
            gainDb = (under - under / juce::jmax (1.0f, ratio));
        }
    }
    else if (under > 0.0f)
    {
        gainDb = (under - under / juce::jmax (1.0f, ratio));
    }

    const float target = juce::Decibels::decibelsToGain (gainDb);
    
    // Much more gradual gain smoothing to prevent sudden jumps
    if (upwardsSmoothGain < 0.5f) // If gain is low, ramp up very slowly
    {
        upwardsSmoothGain = upwardsSmoothGain * 0.98f + target * 0.02f;
    }
    else
    {
        if (target > upwardsSmoothGain) upwardsSmoothGain = upwardsSmoothGain * upwardsAttackCoeff  + target * (1.0f - upwardsAttackCoeff);
        else                            upwardsSmoothGain = upwardsSmoothGain * upwardsReleaseCoeff + target * (1.0f - upwardsReleaseCoeff);
    }

    currentUpwardsGaindB.store (juce::jlimit (0.0f, 20.0f, juce::Decibels::gainToDecibels (upwardsSmoothGain + 1.0e-9f)));
    return upwardsSmoothGain;
}

void CompressorPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();

    updateTimeConstants();
    updateSidechainEQ();

    // Check if vocal mode and drumbus mode are enabled (mutually exclusive)
    vocalModeEnabled = apvts.getRawParameterValue ("VOCAL_MODE")->load() > 0.5f;
    drumbusModeEnabled = apvts.getRawParameterValue ("DRUMBUS_MODE")->load() > 0.5f && !vocalModeEnabled;
    
    // Reset envelope followers if they're in an invalid state to prevent pops
    if (env < 1.0e-12f) env = 1.0e-12f;
    if (upwardsEnv < 1.0e-12f) upwardsEnv = 1.0e-12f;
    
    // Check processing order
    bool upwardsFirst = apvts.getRawParameterValue ("UPWARDS_FIRST")->load() > 0.5f;
    
    {
        // Standard processing without oversampling
        // Input gain
        const float inGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("INPUT_GAIN")->load());
        buffer.applyGain (inGain);
        
        // Calculate input level (after input gain) and detect audio activity
        float inputPeak = 0.0f;
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float* inputData = buffer.getReadPointer(ch);
            for (int n = 0; n < numSamples; ++n)
            {
                inputPeak = juce::jmax(inputPeak, std::abs(inputData[n]));
            }
        }
        inputLevel = inputPeak > 0.0f ? juce::Decibels::gainToDecibels(inputPeak) : -60.0f;
        
        // Detect audio activity (threshold at -60dB)
        const bool hasAudio = inputPeak > 1.0e-3f; // -60dB threshold
        if (hasAudio)
        {
            audioInactiveCounter = 0; // Reset inactive counter
            if (!audioIsActive)
            {
                audioIsActive = true;
                upwardsStartupDelay = 0; // Reset startup delay when audio starts
            }
        }
        else
        {
            audioInactiveCounter += numSamples;
            if (audioInactiveCounter > DEACTIVATION_THRESHOLD)
            {
                audioIsActive = false; // Deactivate after 50ms of silence
                upwardsStartupDelay = 0; // Reset startup delay
                // Reset upwards compressor state when audio becomes inactive
                upwardsEnv = 1.0e-12f;
                upwardsSmoothGain = 1.0f;
                upwardsInitialRamp = true;
                currentUpwardsGaindB.store(0.0f);
            }
        }

        // Wet copy
        wetBuffer.makeCopyOf (buffer, true);

        // Build internal sidechain: sum main input to mono
        scBuffer.clear();
        float* scWrite = scBuffer.getWritePointer (0);
        for (int n = 0; n < numSamples; ++n)
        {
            float s = 0.0f;
            for (int ch = 0; ch < numCh; ++ch)
                s += buffer.getReadPointer (ch)[n];
            scWrite[n] = s * (1.0f / juce::jmax (1, numCh));
        }

        // Apply detector EQ
        float* scData = scBuffer.getWritePointer (0);
        for (int n = 0; n < numSamples; ++n)
            scData[n] = scEQ.processSample(scData[n]);

        // Process based on order with proper cascading
        if (upwardsFirst)
        {
            // Upwards compressor first
            const bool upwardsBypass = apvts.getRawParameterValue ("UPWARDS_BYPASS")->load();
            if (!upwardsBypass && audioIsActive) // Process if NOT bypassed AND audio is active
            {
                // Add startup delay to prevent initial surge
                if (upwardsStartupDelay < ACTIVATION_DELAY_SAMPLES) // Wait for 100ms at 44.1kHz
                {
                    upwardsStartupDelay += numSamples;
                    // During startup delay, just pass through without processing
                }
                else
                {
                    const float* scRead = scBuffer.getReadPointer (0);
                    const float upwardsMix = apvts.getRawParameterValue ("UPWARDS_MIX")->load() * 0.01f; // 0..1
                    
                    for (int n = 0; n < numSamples; ++n)
                    {
                        const float g = computeUpwardsGain (scRead[n]);
                        for (int ch = 0; ch < numCh; ++ch)
                        {
                            const float originalSample = wetBuffer.getReadPointer (ch)[n];
                            const float processedSample = originalSample * g;
                            wetBuffer.getWritePointer (ch)[n] = originalSample * (1.0f - upwardsMix) + processedSample * upwardsMix;
                        }
                    }
                }
                
                // Apply upwards output gain (feeds into downwards compressor)
                const float upwardsOutputGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("UPWARDS_OUTPUT")->load());
                for (int ch = 0; ch < numCh; ++ch)
                    wetBuffer.applyGain (ch, 0, numSamples, upwardsOutputGain);
            }
            
            // Update sidechain for downwards compressor to use processed signal
            scBuffer.clear();
            float* scWrite = scBuffer.getWritePointer (0);
            for (int n = 0; n < numSamples; ++n)
            {
                float s = 0.0f;
                for (int ch = 0; ch < numCh; ++ch)
                    s += wetBuffer.getReadPointer (ch)[n];
                scWrite[n] = s * (1.0f / juce::jmax (1, numCh));
            }
            
            // Apply detector EQ to updated sidechain
            float* scData = scBuffer.getWritePointer (0);
            for (int n = 0; n < numSamples; ++n)
                scData[n] = scEQ.processSample(scData[n]);
            
            // Then downwards compressor (processes the output of upwards compressor)
            const bool downwardsBypass = apvts.getRawParameterValue ("DOWNWARDS_BYPASS")->load();
            if (!downwardsBypass) // Process if NOT bypassed
            {
                const float downwardsMix = apvts.getRawParameterValue ("MIX")->load() * 0.01f; // 0..1
                for (int n = 0; n < numSamples; ++n)
                {
                    const float g = computeGain (scData[n]);
                    for (int ch = 0; ch < numCh; ++ch)
                    {
                        const float originalSample = wetBuffer.getReadPointer (ch)[n];
                        const float processedSample = originalSample * g;
                        wetBuffer.getWritePointer (ch)[n] = originalSample * (1.0f - downwardsMix) + processedSample * downwardsMix;
                    }
                }
                
                // Apply downwards output gain (feeds into global mix)
                const float downwardsOutputGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("DOWNWARDS_OUTPUT")->load());
                for (int ch = 0; ch < numCh; ++ch)
                    wetBuffer.applyGain (ch, 0, numSamples, downwardsOutputGain);
            }
        }
        else
        {
            // Downwards compressor first
            const bool downwardsBypass = apvts.getRawParameterValue ("DOWNWARDS_BYPASS")->load();
            if (!downwardsBypass) // Process if NOT bypassed
            {
                const float* scRead = scBuffer.getReadPointer (0);
                const float downwardsMix = apvts.getRawParameterValue ("MIX")->load() * 0.01f; // 0..1
                
                for (int n = 0; n < numSamples; ++n)
                {
                    const float g = computeGain (scRead[n]);
                    for (int ch = 0; ch < numCh; ++ch)
                    {
                        const float originalSample = wetBuffer.getReadPointer (ch)[n];
                        const float processedSample = originalSample * g;
                        wetBuffer.getWritePointer (ch)[n] = originalSample * (1.0f - downwardsMix) + processedSample * downwardsMix;
                    }
                }
                
                // Apply downwards output gain (feeds into upwards compressor)
                const float downwardsOutputGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("DOWNWARDS_OUTPUT")->load());
                for (int ch = 0; ch < numCh; ++ch)
                    wetBuffer.applyGain (ch, 0, numSamples, downwardsOutputGain);
            }
            
            // Update sidechain for upwards compressor to use processed signal
            scBuffer.clear();
            float* scWrite = scBuffer.getWritePointer (0);
            for (int n = 0; n < numSamples; ++n)
            {
                float s = 0.0f;
                for (int ch = 0; ch < numCh; ++ch)
                    s += wetBuffer.getReadPointer (ch)[n];
                scWrite[n] = s * (1.0f / juce::jmax (1, numCh));
            }
            
            // Apply detector EQ to updated sidechain
            float* scData = scBuffer.getWritePointer (0);
            for (int n = 0; n < numSamples; ++n)
                scData[n] = scEQ.processSample(scData[n]);
            
            // Then upwards compressor (processes the output of downwards compressor)
            const bool upwardsBypass = apvts.getRawParameterValue ("UPWARDS_BYPASS")->load();
            if (!upwardsBypass && audioIsActive) // Process if NOT bypassed AND audio is active
            {
                // Add startup delay to prevent initial surge
                if (upwardsStartupDelay < ACTIVATION_DELAY_SAMPLES) // Wait for 100ms at 44.1kHz
                {
                    upwardsStartupDelay += numSamples;
                    // During startup delay, just pass through without processing
                }
                else
                {
                    const float upwardsMix = apvts.getRawParameterValue ("UPWARDS_MIX")->load() * 0.01f; // 0..1
                    for (int n = 0; n < numSamples; ++n)
                    {
                        const float g = computeUpwardsGain (scData[n]);
                        for (int ch = 0; ch < numCh; ++ch)
                        {
                            const float originalSample = wetBuffer.getReadPointer (ch)[n];
                            const float processedSample = originalSample * g;
                            wetBuffer.getWritePointer (ch)[n] = originalSample * (1.0f - upwardsMix) + processedSample * upwardsMix;
                        }
                    }
                }
                
                // Apply upwards output gain (feeds into global mix)
                const float upwardsOutputGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("UPWARDS_OUTPUT")->load());
                for (int ch = 0; ch < numCh; ++ch)
                    wetBuffer.applyGain (ch, 0, numSamples, upwardsOutputGain);
            }
        }

        // Apply global mix (wet/dry blend)
        const float globalMix = apvts.getRawParameterValue ("GLOBAL_MIX")->load() * 0.01f; // 0..1
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float* dryData = buffer.getReadPointer(ch);
            const float* wetData = wetBuffer.getReadPointer(ch);
            float* outputData = buffer.getWritePointer(ch);
            
            for (int n = 0; n < numSamples; ++n)
            {
                outputData[n] = dryData[n] * (1.0f - globalMix) + wetData[n] * globalMix;
            }
        }

        // Apply global output gain (after global mix)
        const float outGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("OUTPUT_GAIN")->load());
        buffer.applyGain (outGain);
        
        // Calculate output level (after all processing)
        float outputPeak = 0.0f;
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float* outputData = buffer.getReadPointer(ch);
            for (int n = 0; n < numSamples; ++n)
            {
                outputPeak = juce::jmax(outputPeak, std::abs(outputData[n]));
            }
        }
        outputLevel = outputPeak > 0.0f ? juce::Decibels::gainToDecibels(outputPeak) : -60.0f;
    }
}

//==============================================================================
const juce::String CompressorPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CompressorPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CompressorPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CompressorPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CompressorPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CompressorPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CompressorPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CompressorPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String CompressorPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void CompressorPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

bool CompressorPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

//==============================================================================
juce::AudioProcessorEditor* CompressorPluginAudioProcessor::createEditor()
{
    return new CompressorPluginAudioProcessorEditor (*this);
}

void CompressorPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
    apvts.state.writeToStream (mos);
}

void CompressorPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) sizeInBytes);
    if (tree.isValid()) apvts.replaceState (tree);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CompressorPluginAudioProcessor::createParameterLayout()
{
    using R = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("INPUT_GAIN",  "Input Gain",  R (-24.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("OUTPUT_GAIN", "Output Gain", R (-24.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("GLOBAL_MIX",  "Global Mix",  R (0.0f, 100.0f, 0.1f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("THRESHOLD",   "Threshold",  R (-60.0f, 0.0f, 0.01f), -24.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RATIO",       "Ratio",      R (1.0f, 20.0f, 0.01f), 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK",      "Attack",     R (0.1f, 100.0f, 0.01f), 10.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE",     "Release",    R (5.0f, 1000.0f, 0.01f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("KNEE",        "Knee",       R (0.0f, 24.0f, 0.01f), 6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MIX",         "Mix",        R (0.0f, 100.0f, 0.01f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DOWNWARDS_OUTPUT", "Downwards Output", R (-24.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("DOWNWARDS_BYPASS", "Downwards Bypass", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("VOCAL_MODE",   "Vocal Mode", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("DRUMBUS_MODE", "Drumbus Mode", false));
    
    // Upwards compressor parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UPWARDS_THRESHOLD",   "Upwards Threshold",  R (-60.0f, 0.0f, 0.01f), -40.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UPWARDS_RATIO",       "Upwards Ratio",      R (1.0f, 10.0f, 0.01f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UPWARDS_ATTACK",      "Upwards Attack",     R (0.1f, 100.0f, 0.01f), 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UPWARDS_RELEASE",     "Upwards Release",    R (5.0f, 1000.0f, 0.01f), 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UPWARDS_KNEE",        "Upwards Knee",       R (0.0f, 24.0f, 0.01f), 3.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UPWARDS_MIX",         "Upwards Mix",        R (0.0f, 100.0f, 0.1f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UPWARDS_OUTPUT",      "Upwards Output",     R (-24.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("UPWARDS_BYPASS",      "Upwards Bypass", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("UPWARDS_FIRST",        "Upwards First", false));

    return { params.begin(), params.end() };
}

// Factory for old-style hosts / standalone
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorPluginAudioProcessor();
}


