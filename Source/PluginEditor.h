#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class GRMeter : public juce::Component
{
public:
    enum MeterType
    {
        InputOutput,
        GainReduction,
        UpwardsGain
    };
    
    GRMeter() : meterType(InputOutput) {}
    
    void setValue (float grDbPositive) { value = juce::jlimit (0.0f, 60.0f, grDbPositive); meterType = GainReduction; repaint(); }
    void setInputValue (float inputDb) { value = juce::jlimit (-60.0f, 6.0f, inputDb); meterType = InputOutput; repaint(); }
    void setOutputValue (float outputDb) { value = juce::jlimit (-60.0f, 6.0f, outputDb); meterType = InputOutput; repaint(); }
    void setUpwardsGainValue (float gainDbPositive) { value = juce::jlimit (0.0f, 20.0f, gainDbPositive); meterType = UpwardsGain; repaint(); }
    
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().reduced (4);
        g.setColour (juce::Colours::black.withAlpha (0.7f));
        g.fillRoundedRectangle (bounds.toFloat(), 8.0f);
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.drawRoundedRectangle (bounds.toFloat(), 8.0f, 1.0f);

        // Meter area inside
        auto meter = bounds.reduced (12);

        // Use meter type to determine rendering behavior
        bool isUpwardsGainMeter = (meterType == UpwardsGain);
        bool isGainReductionMeter = (meterType == GainReduction);
        
        if (isUpwardsGainMeter) {
            // Upwards gain meter: 0 to 20dB scale, 0dB at bottom, fill upward
            const float maxShow = 20.0f;
            const float ticks[] = { 0.0f, 5.0f, 10.0f, 15.0f, 20.0f };
            
            for (float t : ticks)
            {
                const float normalizedValue = (maxShow - t) / maxShow; // 0dB at bottom, 20dB at top
                const float y = (float) meter.getY() + normalizedValue * (float) meter.getHeight();
                g.setColour (juce::Colours::white.withAlpha (0.2f));
                g.drawLine ((float) meter.getX(), y, (float) meter.getRight(), y, 1.0f);
                juce::Rectangle<int> label ((int) meter.getRight() - 44, (int) y - 8, 44, 16);
                g.setColour (juce::Colours::white.withAlpha (0.6f));
                g.setFont (12.0f);
                g.drawFittedText (juce::String ("+") + juce::String ((int) t) + " dB",
                                  label, juce::Justification::centredLeft, 1);
            }
            
            // Bar fill for upwards gain meter (fill upward)
            const float displayValue = juce::jlimit (0.0f, maxShow, value);
            const float normalizedValue = displayValue / maxShow;
            const int barHeight = (int) std::round (normalizedValue * (float) meter.getHeight());
            juce::Rectangle<int> fillRect = meter.withY (meter.getBottom() - barHeight).withHeight (barHeight);
            g.setColour (juce::Colours::lightgreen.withAlpha (0.9f));
            g.fillRoundedRectangle (fillRect.toFloat(), 6.0f);
        } else if (!isGainReductionMeter) {
            // Input/Output meters: +6dB to -30dB scale, fill upward
            const float maxShow = 36.0f; // +6 to -30 = 36dB range
            const float ticks[] = { 6.0f, 0.0f, -6.0f, -12.0f, -20.0f, -30.0f };
            
            for (float t : ticks)
            {
                const float normalizedValue = (6.0f - t) / maxShow; // +6dB at top, -30dB at bottom
                const float y = (float) meter.getY() + normalizedValue * (float) meter.getHeight();
                g.setColour (juce::Colours::white.withAlpha (0.2f));
                g.drawLine ((float) meter.getX(), y, (float) meter.getRight(), y, 1.0f);
                juce::Rectangle<int> label ((int) meter.getRight() - 44, (int) y - 8, 44, 16);
                g.setColour (juce::Colours::white.withAlpha (0.6f));
                g.setFont (12.0f);
                g.drawFittedText (juce::String ((int) t) + " dB",
                                  label, juce::Justification::centredLeft, 1);
            }
            
            // Bar fill for input/output meters (fill upward)
            const float clampedValue = juce::jlimit (-30.0f, 6.0f, value);
            const float normalizedValue = (clampedValue - (-30.0f)) / maxShow; // Higher level = more fill
            const int barHeight = (int) std::round (normalizedValue * (float) meter.getHeight());
            juce::Rectangle<int> fillRect = meter.withY (meter.getBottom() - barHeight).withHeight (barHeight);
            
            // Change color to red when exceeding 0dB
            juce::Colour meterColor = (clampedValue > 0.0f) ? juce::Colours::red : juce::Colours::skyblue;
            g.setColour (meterColor.withAlpha (0.9f));
            g.fillRoundedRectangle (fillRect.toFloat(), 6.0f);
        } else {
            // Gain reduction meter: Always show -40dB to 0dB scale, 0dB at top, -40dB at bottom, fill from top
            const float maxShow = 40.0f;
            const float ticks[] = { 0.0f, -5.0f, -10.0f, -20.0f, -30.0f, -40.0f };
            
            for (float t : ticks)
            {
                const float normalizedValue = (0.0f - t) / maxShow; // 0dB at top, -40dB at bottom
                const float y = (float) meter.getY() + normalizedValue * (float) meter.getHeight();
                g.setColour (juce::Colours::white.withAlpha (0.2f));
                g.drawLine ((float) meter.getX(), y, (float) meter.getRight(), y, 1.0f);
                juce::Rectangle<int> label ((int) meter.getRight() - 44, (int) y - 8, 44, 16);
                g.setColour (juce::Colours::white.withAlpha (0.6f));
                g.setFont (12.0f);
                g.drawFittedText (juce::String ((int) t) + " dB",
                                  label, juce::Justification::centredLeft, 1);
            }
            
            // Bar fill for gain reduction (fill from top down)
            const float displayValue = juce::jlimit (0.0f, maxShow, value);
            const int barHeight = (int) std::round (displayValue / maxShow * (float) meter.getHeight());
            juce::Rectangle<int> fillRect = meter.withY (meter.getY()).withHeight (barHeight);
            g.setColour (juce::Colours::red.withAlpha (0.9f));
            g.fillRoundedRectangle (fillRect.toFloat(), 6.0f);
        }
    }
private:
    float value = 0.0f;
    MeterType meterType;
};

class CompressorPluginAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                              private juce::Timer,
                                              private juce::Value::Listener
{
public:
    explicit CompressorPluginAudioProcessorEditor (CompressorPluginAudioProcessor&);
    ~CompressorPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void valueChanged(juce::Value& value) override;
    void setupSlider(juce::Slider&, const juce::String&, bool isDial = true);
    void setupLabel(juce::Label&, const juce::String&);
    void setupSectionLabel(juce::Label&, const juce::String&);
    
    // Override to set minimum size constraint


    CompressorPluginAudioProcessor& processor;

    // Header controls
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::Slider globalMixSlider;
    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider kneeSlider;
    juce::Slider mixSlider;
    juce::Slider downwardsOutputSlider;
    juce::ToggleButton vocalModeButton;
    juce::ToggleButton drumbusModeButton;

    // Upwards compressor controls
    juce::Slider upwardsThresholdSlider;
    juce::Slider upwardsRatioSlider;
    juce::Slider upwardsAttackSlider;
    juce::Slider upwardsReleaseSlider;
    juce::Slider upwardsKneeSlider;
    juce::Label upwardsKneeLabel;
    juce::Slider upwardsMixSlider;
    juce::Label upwardsMixLabel;
    juce::Slider upwardsOutputSlider;
    juce::Label upwardsOutputLabel;
    juce::ToggleButton upwardsFirstButton;

    // Labels for header knobs
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;
    juce::Label globalMixLabel;
    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label kneeLabel;
    juce::Label mixLabel;
    juce::Label downwardsOutputLabel;

    // Labels for upwards compressor knobs
    juce::Label upwardsThresholdLabel;
    juce::Label upwardsRatioLabel;
    juce::Label upwardsAttackLabel;
    juce::Label upwardsReleaseLabel;

    // Meters
    GRMeter grMeter;
    GRMeter inputMeter;
    GRMeter outputMeter;
    GRMeter upwardsMeter;
    
    // Meter labels
    juce::Label inputMeterLabel;
    juce::Label outputMeterLabel;
    juce::Label grMeterLabel;
    juce::Label upwardsMeterLabel;

    // Section labels
    juce::Label downwardsSectionLabel;
    juce::Label upwardsSectionLabel;
    
    // Bypass buttons
    juce::ToggleButton downwardsBypassButton;
    juce::ToggleButton upwardsBypassButton;

    // Attachments for downwards compressor
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> globalMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> downwardsOutputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vocalModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> drumbusModeAttachment;

    // Attachments for upwards compressor
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upwardsThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upwardsRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upwardsAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upwardsReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upwardsKneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upwardsMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upwardsOutputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> upwardsFirstAttachment;
    // Custom bypass button handling
    juce::Value downwardsBypassValue;
    juce::Value upwardsBypassValue;
    
    // Meter smoothing variables
    float smoothedInputLevel = -60.0f;
    float smoothedOutputLevel = -60.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorPluginAudioProcessorEditor)
};
