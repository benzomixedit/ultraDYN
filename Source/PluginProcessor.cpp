#include "PluginProcessor.h"

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
{
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

    scEQ.reset();

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
    // Nothing to do here
}

void CompressorPluginAudioProcessor::updateTimeConstants()
{
    const double sr = juce::jmax (1.0, getSampleRate());
    // tiny +1 inside to avoid zero divisions in pathological cases
    attackCoeff  = std::exp (-1.0f / ((float) (attack  * 0.001 * sr) + 1.0f));
    releaseCoeff = std::exp (-1.0f / ((float) (release * 0.001 * sr) + 1.0f));

    // Upwards compressor time constants
    upwardsAttackCoeff  = std::exp (-1.0f / ((float) (upwardsAttack  * 0.001 * sr) + 1.0f));
    upwardsReleaseCoeff = std::exp (-1.0f / ((float) (upwardsRelease * 0.001 * sr) + 1.0f));
}

void CompressorPluginAudioProcessor::updateSidechainEQ()
{
    // Sidechain EQ modes: boost or cut at 1.5 kHz for different compression focuses
    const double sr = juce::jmax (1.0, getSampleRate());
    const double freq = 1500.0;
    const float  q    = 0.7071f; // wide, musical Q
    
    if (vocalMode)
    {
        // Vocal mode: threshold-coupled peak gain, up to +5 dB as threshold lowers
        const float thresholdMin = -60.0f;
        const float thresholdMax = 0.0f;
        const float thr = threshold;
        const float normalizedThr = (thr - thresholdMin) / (thresholdMax - thresholdMin);
        const float peakGain = normalizedThr * 5.0f; // 0 to +5 dB
        scEQ.setPeak (sr, freq, q, peakGain);
    }
    else if (drumbusMode)
    {
        // Drumbus mode: fixed cut at -3 dB to reduce harshness
        scEQ.setPeak (sr, freq, q, -3.0f);
    }
    else
    {
        // Normal mode: no EQ
        scEQ.setPeak (sr, freq, q, 0.0f);
    }
}

float CompressorPluginAudioProcessor::processDownwardsCompressor(float input, int channel)
{
    // Sidechain EQ processing (mono detector)
    float scSample = scEQ.process (input);
    
    // RMS envelope follower
    const float squared = scSample * scSample;
    env = env * 0.999f + squared * 0.001f; // Very slow RMS averaging
    
    // Convert to dB
    const float dbInput = env > 1.0e-12f ? juce::Decibels::gainToDecibels (std::sqrt (env)) : -60.0f;
    
    // Compute gain reduction
    float gainReduction = 0.0f;
    if (!downwardsBypass && dbInput > threshold)
    {
        const float overThreshold = dbInput - threshold;
        const float kneeWidth = knee;
        
        if (overThreshold < kneeWidth)
        {
            // Soft knee
            const float kneeRatio = 1.0f + (ratio - 1.0f) * overThreshold / kneeWidth;
            gainReduction = overThreshold * (1.0f - 1.0f / kneeRatio);
        }
        else
        {
            // Hard knee
            gainReduction = overThreshold * (1.0f - 1.0f / ratio);
        }
    }
    
    // Smooth gain reduction
    const float targetGain = juce::Decibels::decibelsToGain (-gainReduction);
    if (targetGain < smoothGain) 
        smoothGain = smoothGain * attackCoeff + targetGain * (1.0f - attackCoeff);
    else 
        smoothGain = smoothGain * releaseCoeff + targetGain * (1.0f - releaseCoeff);
    
    // Store gain reduction for metering
    currentGRdB.store (gainReduction);
    
    // Apply gain and mix
    const float processed = input * smoothGain;
    const float mixAmount = mix * 0.01f;
    return input * (1.0f - mixAmount) + processed * mixAmount;
}

float CompressorPluginAudioProcessor::processUpwardsCompressor(float input, int channel)
{
    // Sidechain EQ processing (mono detector)
    float scSample = scEQ.process (input);
    
    // RMS envelope follower
    const float squared = scSample * scSample;
    upwardsEnv = upwardsEnv * 0.999f + squared * 0.001f; // Very slow RMS averaging
    
    // Convert to dB
    const float dbInput = upwardsEnv > 1.0e-12f ? juce::Decibels::gainToDecibels (std::sqrt (upwardsEnv)) : -60.0f;
    
    // Compute gain boost
    float gainBoost = 0.0f;
    if (!upwardsBypass && dbInput < upwardsThreshold)
    {
        const float underThreshold = upwardsThreshold - dbInput;
        const float kneeWidth = upwardsKnee;
        
        if (underThreshold < kneeWidth)
        {
            // Soft knee
            const float kneeRatio = 1.0f + (upwardsRatio - 1.0f) * underThreshold / kneeWidth;
            gainBoost = underThreshold * (1.0f - 1.0f / kneeRatio);
        }
        else
        {
            // Hard knee
            gainBoost = underThreshold * (1.0f - 1.0f / upwardsRatio);
        }
    }
    
    // Smooth gain boost with initial ramp protection
    const float targetGain = juce::Decibels::decibelsToGain (gainBoost);
    float target = targetGain;
    
    // Initial ramp protection for upwards compressor
    if (upwardsInitialRamp)
    {
        upwardsStartupDelay++;
        if (upwardsStartupDelay > 441) // 10ms at 44.1kHz
        {
            upwardsInitialRamp = false;
        }
        else
        {
            target = 1.0f; // No boost during initial ramp
        }
    }
    
    // Much more gradual gain smoothing to prevent sudden jumps
    if (upwardsSmoothGain < 0.5f) // If gain is low, ramp up very slowly
    {
        upwardsSmoothGain = upwardsSmoothGain * 0.98f + target * 0.02f;
    }
    else
    {
        if (target > upwardsSmoothGain) 
            upwardsSmoothGain = upwardsSmoothGain * upwardsAttackCoeff + target * (1.0f - upwardsAttackCoeff);
        else 
            upwardsSmoothGain = upwardsSmoothGain * upwardsReleaseCoeff + target * (1.0f - upwardsReleaseCoeff);
    }

    currentUpwardsGaindB.store (juce::jlimit (0.0f, 20.0f, juce::Decibels::gainToDecibels (upwardsSmoothGain + 1.0e-9f)));
    return input * upwardsSmoothGain;
}

void CompressorPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();

    updateTimeConstants();
    updateSidechainEQ();

    // Reset envelope followers if they're in an invalid state to prevent pops
    if (env < 1.0e-12f) env = 1.0e-12f;
    if (upwardsEnv < 1.0e-12f) upwardsEnv = 1.0e-12f;
    
    // Check processing order
    bool processUpwardsFirst = upwardsFirst;
    
    {
        // Input gain
        const float inGain = juce::Decibels::decibelsToGain (inputGain);
        buffer.applyGain (inGain);
        
        // Calculate input level
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
        
        // Copy to wet buffer
        wetBuffer.makeCopyOf (buffer);
        
        // Process each channel
        for (int ch = 0; ch < numCh; ++ch)
        {
            float* channelData = wetBuffer.getWritePointer(ch);
            
            for (int n = 0; n < numSamples; ++n)
            {
                float sample = channelData[n];
                
                if (processUpwardsFirst)
                {
                    // Upwards first, then downwards
                    sample = processUpwardsCompressor(sample, ch);
                    sample = processDownwardsCompressor(sample, ch);
                }
                else
                {
                    // Downwards first, then upwards
                    sample = processDownwardsCompressor(sample, ch);
                    sample = processUpwardsCompressor(sample, ch);
                }
                
                channelData[n] = sample;
            }
        }
        
        // Apply downwards output gain
        const float downwardsOutputGain = juce::Decibels::decibelsToGain (downwardsOutput);
        wetBuffer.applyGain (downwardsOutputGain);
        
        // Apply upwards output gain (feeds into global mix)
        const float upwardsOutputGain = juce::Decibels::decibelsToGain (upwardsOutput);
        for (int ch = 0; ch < numCh; ++ch)
            wetBuffer.applyGain (ch, 0, numSamples, upwardsOutputGain);
    }

    // Apply global mix (wet/dry blend)
    const float globalMixAmount = globalMix * 0.01f; // 0..1
    for (int ch = 0; ch < numCh; ++ch)
    {
        const float* dryData = buffer.getReadPointer(ch);
        const float* wetData = wetBuffer.getReadPointer(ch);
        float* outputData = buffer.getWritePointer(ch);
        
        for (int n = 0; n < numSamples; ++n)
        {
            outputData[n] = dryData[n] * (1.0f - globalMixAmount) + wetData[n] * globalMixAmount;
        }
    }

    // Apply global output gain (after global mix)
    const float outGain = juce::Decibels::decibelsToGain (outputGain);
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

//==============================================================================
juce::AudioProcessorEditor* CompressorPluginAudioProcessor::createEditor()
{
    return nullptr; // Headless plugin - no GUI
}

void CompressorPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Simple state saving - just save parameter values
    juce::MemoryOutputStream mos (destData, true);
    mos.writeFloat (inputGain);
    mos.writeFloat (outputGain);
    mos.writeFloat (globalMix);
    mos.writeFloat (threshold);
    mos.writeFloat (ratio);
    mos.writeFloat (attack);
    mos.writeFloat (release);
    mos.writeFloat (knee);
    mos.writeFloat (mix);
    mos.writeFloat (downwardsOutput);
    mos.writeBool (downwardsBypass);
    mos.writeBool (vocalMode);
    mos.writeBool (drumbusMode);
    mos.writeFloat (upwardsThreshold);
    mos.writeFloat (upwardsRatio);
    mos.writeFloat (upwardsAttack);
    mos.writeFloat (upwardsRelease);
    mos.writeFloat (upwardsKnee);
    mos.writeFloat (upwardsMix);
    mos.writeFloat (upwardsOutput);
    mos.writeBool (upwardsBypass);
    mos.writeBool (upwardsFirst);
}

void CompressorPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Simple state loading - just load parameter values
    juce::MemoryInputStream mis (data, sizeInBytes, false);
    inputGain = mis.readFloat();
    outputGain = mis.readFloat();
    globalMix = mis.readFloat();
    threshold = mis.readFloat();
    ratio = mis.readFloat();
    attack = mis.readFloat();
    release = mis.readFloat();
    knee = mis.readFloat();
    mix = mis.readFloat();
    downwardsOutput = mis.readFloat();
    downwardsBypass = mis.readBool();
    vocalMode = mis.readBool();
    drumbusMode = mis.readBool();
    upwardsThreshold = mis.readFloat();
    upwardsRatio = mis.readFloat();
    upwardsAttack = mis.readFloat();
    upwardsRelease = mis.readFloat();
    upwardsKnee = mis.readFloat();
    upwardsMix = mis.readFloat();
    upwardsOutput = mis.readFloat();
    upwardsBypass = mis.readBool();
    upwardsFirst = mis.readBool();
}

//==============================================================================
// Factory for old-style hosts / standalone
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorPluginAudioProcessor();
}

