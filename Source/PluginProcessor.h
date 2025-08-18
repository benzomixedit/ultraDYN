#pragma once

#include <JuceHeader.h>
#include <cmath>

class CompressorPluginAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    CompressorPluginAudioProcessor();
    ~CompressorPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return "SidechainEQCompressor"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Public API for UI
    float getGainReduction() const noexcept { return currentGRdB.load(); } // positive dB value (e.g., 6.2)
    float getInputLevel() const noexcept { return inputLevel; }
    float getOutputLevel() const noexcept { return outputLevel; }
    float getUpwardsGain() const noexcept { return currentUpwardsGaindB.load(); } // positive dB value (e.g., 3.1)
    bool isDownwardsBypassed() const noexcept { return apvts.getRawParameterValue("DOWNWARDS_BYPASS")->load() > 0.5f; }
    bool isUpwardsBypassed() const noexcept { return apvts.getRawParameterValue("UPWARDS_BYPASS")->load() > 0.5f; }

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    //==============================================================================
    // Simple RBJ peaking biquad for sidechain EQ (detector only)
    struct Biquad
    {
        float b0{1.0f}, b1{0.0f}, b2{0.0f}, a1{0.0f}, a2{0.0f};
        float z1{0.0f}, z2{0.0f};

        void reset() noexcept { z1 = z2 = 0.0f; }

        void setPeak (double sr, double freq, float Q, float gainDb) noexcept
        {
            const float A    = std::pow (10.0f, gainDb * 0.025f); // 10^(dB/40)
            const float w0   = juce::MathConstants<float>::twoPi * (float) freq / (float) sr;
            const float cw0  = std::cos (w0);
            const float sw0  = std::sin (w0);
            const float alpha = sw0 / (2.0f * juce::jmax (1.0e-6f, Q));

            const float b0n = 1.0f + alpha * A;
            const float b1n = -2.0f * cw0;
            const float b2n = 1.0f - alpha * A;
            const float a0n = 1.0f + alpha / A;
            const float a1n = -2.0f * cw0;
            const float a2n = 1.0f - alpha / A;

            const float invA0 = 1.0f / a0n;
            b0 = b0n * invA0;
            b1 = b1n * invA0;
            b2 = b2n * invA0;
            a1 = a1n * invA0;
            a2 = a2n * invA0;
        }

        inline float process (float x) noexcept
        {
            const float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };

    // Vocal mode and drumbus mode for sidechain EQ
    bool vocalModeEnabled = false;
    bool drumbusModeEnabled = false;
    
    // Level tracking for meters
    float inputLevel = -60.0f;
    float outputLevel = -60.0f;

    // Parameters
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "PARAMS", createParameterLayout() };

    // Sidechain EQ for detector path (peak @ 1.5 kHz)
    Biquad scEQ;

    // Envelope follower (RMS) and gain computer state for downwards compressor
    float env = 0.0f;        // RMS detector (squared average)
    float smoothGain = 1.0f; // smoothed linear gain for attack/release

    // Smoothers for attack/release (per-sample coefficients) for downwards compressor
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // GR meter for downwards compressor
    std::atomic<float> currentGRdB { 0.0f }; // store as positive dB reduction

    // Upwards compressor state
    float upwardsEnv = 0.0f;        // RMS detector for upwards compressor
    float upwardsSmoothGain = 1.0f; // smoothed linear gain for upwards compressor
    float upwardsAttackCoeff = 0.0f;
    float upwardsReleaseCoeff = 0.0f;
    std::atomic<float> currentUpwardsGaindB { 0.0f }; // store as positive dB gain
    bool upwardsInitialRamp = true; // Track if we're in initial ramp mode
    int upwardsStartupDelay = 0; // Delay counter to prevent immediate processing
    bool audioIsActive = false; // Track if audio is currently being processed
    int audioInactiveCounter = 0; // Counter for detecting when audio stops
    static const int ACTIVATION_DELAY_SAMPLES = 441; // 10ms at 44.1kHz
    static const int DEACTIVATION_THRESHOLD = 2205; // 50ms of silence to deactivate

    // Scratch buffers
    juce::AudioBuffer<float> wetBuffer;
    juce::AudioBuffer<float> scBuffer; // mono detector buffer

    // Helpers
    void updateSidechainEQ();
    void updateTimeConstants();
    float computeGain (float scSample) noexcept; // returns linear gain for downwards compressor
    float computeUpwardsGain (float scSample) noexcept; // returns linear gain for upwards compressor

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorPluginAudioProcessor)
};
