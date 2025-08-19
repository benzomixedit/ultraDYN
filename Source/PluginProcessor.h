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
    bool hasEditor() const override { return false; } // Headless plugin

    //==============================================================================
    const juce::String getName() const override { return "ultraDYN"; }

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

    // Public API for parameters
    float getGainReduction() const noexcept { return currentGRdB.load(); }
    float getInputLevel() const noexcept { return inputLevel; }
    float getOutputLevel() const noexcept { return outputLevel; }
    float getUpwardsGain() const noexcept { return currentUpwardsGaindB.load(); }
    bool isDownwardsBypassed() const noexcept { return downwardsBypass; }
    bool isUpwardsBypassed() const noexcept { return upwardsBypass; }

    // Parameter getters
    float getInputGain() const { return inputGain; }
    float getOutputGain() const { return outputGain; }
    float getGlobalMix() const { return globalMix; }
    float getThreshold() const { return threshold; }
    float getRatio() const { return ratio; }
    float getAttack() const { return attack; }
    float getRelease() const { return release; }
    float getKnee() const { return knee; }
    float getMix() const { return mix; }
    float getDownwardsOutput() const { return downwardsOutput; }
    bool getVocalMode() const { return vocalMode; }
    bool getDrumbusMode() const { return drumbusMode; }
    float getUpwardsThreshold() const { return upwardsThreshold; }
    float getUpwardsRatio() const { return upwardsRatio; }
    float getUpwardsAttack() const { return upwardsAttack; }
    float getUpwardsRelease() const { return upwardsRelease; }
    float getUpwardsKnee() const { return upwardsKnee; }
    float getUpwardsMix() const { return upwardsMix; }
    float getUpwardsOutput() const { return upwardsOutput; }
    bool getUpwardsFirst() const { return upwardsFirst; }

    // Parameter setters
    void setInputGain(float value) { inputGain = value; }
    void setOutputGain(float value) { outputGain = value; }
    void setGlobalMix(float value) { globalMix = value; }
    void setThreshold(float value) { threshold = value; }
    void setRatio(float value) { ratio = value; }
    void setAttack(float value) { attack = value; }
    void setRelease(float value) { release = value; }
    void setKnee(float value) { knee = value; }
    void setMix(float value) { mix = value; }
    void setDownwardsOutput(float value) { downwardsOutput = value; }
    void setVocalMode(bool value) { vocalMode = value; }
    void setDrumbusMode(bool value) { drumbusMode = value; }
    void setUpwardsThreshold(float value) { upwardsThreshold = value; }
    void setUpwardsRatio(float value) { upwardsRatio = value; }
    void setUpwardsAttack(float value) { upwardsAttack = value; }
    void setUpwardsRelease(float value) { upwardsRelease = value; }
    void setUpwardsKnee(float value) { upwardsKnee = value; }
    void setUpwardsMix(float value) { upwardsMix = value; }
    void setUpwardsOutput(float value) { upwardsOutput = value; }
    void setUpwardsFirst(bool value) { upwardsFirst = value; }
    void setDownwardsBypass(bool value) { downwardsBypass = value; }
    void setUpwardsBypass(bool value) { upwardsBypass = value; }

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
    bool vocalMode = false;
    bool drumbusMode = false;
    
    // Level tracking for meters
    float inputLevel = -60.0f;
    float outputLevel = -60.0f;

    // Parameters
    float inputGain = 0.0f;
    float outputGain = 0.0f;
    float globalMix = 100.0f;
    float threshold = -24.0f;
    float ratio = 4.0f;
    float attack = 10.0f;
    float release = 100.0f;
    float knee = 6.0f;
    float mix = 100.0f;
    float downwardsOutput = 0.0f;
    bool downwardsBypass = false;
    float upwardsThreshold = -40.0f;
    float upwardsRatio = 2.0f;
    float upwardsAttack = 5.0f;
    float upwardsRelease = 50.0f;
    float upwardsKnee = 3.0f;
    float upwardsMix = 100.0f;
    float upwardsOutput = 0.0f;
    bool upwardsBypass = false;
    bool upwardsFirst = false;

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

    // Processing buffers
    juce::AudioBuffer<float> wetBuffer;
    juce::AudioBuffer<float> scBuffer;

    // Audio state tracking
    bool audioIsActive = false;
    int audioInactiveCounter = 0;
    bool upwardsInitialRamp = true;
    int upwardsStartupDelay = 0;

    // Helper functions
    void updateTimeConstants();
    void updateSidechainEQ();
    float processDownwardsCompressor(float input, int channel);
    float processUpwardsCompressor(float input, int channel);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorPluginAudioProcessor)
};
