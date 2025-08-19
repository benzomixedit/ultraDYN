#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class CompressorPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CompressorPluginAudioProcessor();
    ~CompressorPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter access
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    
    // Public API for UI
    float getGainReduction() const noexcept { return currentGRdB.load(); } // positive dB value (e.g., 6.2)
    float getInputLevel() const noexcept { return inputLevel.load(); }
    float getOutputLevel() const noexcept { return outputLevel.load(); }
    float getUpwardsGain() const noexcept { return currentUpwardsGaindB.load(); } // positive dB value (e.g., 3.1)
    bool isDownwardsBypassed() const noexcept { return apvts.getRawParameterValue("DOWNWARDS_BYPASS")->load() > 0.5f; }
    bool isUpwardsBypassed() const noexcept { return apvts.getRawParameterValue("UPWARDS_BYPASS")->load() > 0.5f; }
    
    // Level metering
    std::atomic<float> inputLevel{-60.0f};
    std::atomic<float> outputLevel{-60.0f};
    std::atomic<float> currentGRdB{0.0f};
    std::atomic<float> currentUpwardsGaindB{0.0f};

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Buffers
    juce::AudioBuffer<float> wetBuffer;
    juce::AudioBuffer<float> scBuffer;
    
    // Sidechain EQ - using IIRFilter from DSP module
    juce::dsp::IIR::Filter<float> scEQ;
    juce::dsp::IIR::Coefficients<float>::Ptr scEQCoeffs;
    
    // Envelope followers
    float env = 1.0e-12f;
    float upwardsEnv = 1.0e-12f;
    
    // Gain smoothing
    float smoothGain = 1.0f;
    float upwardsSmoothGain = 1.0f;
    
    // Time constants
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float upwardsAttackCoeff = 0.0f;
    float upwardsReleaseCoeff = 0.0f;
    
    // Upwards compressor state
    bool upwardsInitialRamp = true;
    int upwardsStartupDelay = 0;
    bool audioIsActive = false;
    int audioInactiveCounter = 0;
    
    // Mode flags
    bool vocalModeEnabled = false;
    bool drumbusModeEnabled = false;
    
    // Constants
    static constexpr int ACTIVATION_DELAY_SAMPLES = 4410; // 100ms at 44.1kHz
    static constexpr int DEACTIVATION_THRESHOLD = 2205;   // 50ms at 44.1kHz
    
    // Helper functions
    void updateTimeConstants();
    void updateSidechainEQ();
    float computeGain(float scSample) noexcept;
    float computeUpwardsGain(float scSample) noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorPluginAudioProcessor)
};
