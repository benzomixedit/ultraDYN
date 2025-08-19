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
    
    // Sidechain EQ
    juce::IIRFilter scEQ;
    
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
