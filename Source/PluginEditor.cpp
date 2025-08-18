#include "PluginProcessor.h"
#include "PluginEditor.h"

// Suppress deprecation warnings for Font constructor
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

CompressorPluginAudioProcessorEditor::CompressorPluginAudioProcessorEditor (CompressorPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    auto& apvts = processor.getAPVTS();

    // Setup header controls
    setupSlider (inputGainSlider,  "Input");
    setupSlider (outputGainSlider, "Output");
    setupSlider (globalMixSlider,  "Global Mix");
    setupSlider (thresholdSlider,  "Threshold");
    setupSlider (ratioSlider,      "Ratio");
    setupSlider (attackSlider,     "Attack");
    setupSlider (releaseSlider,    "Release");
    setupSlider (kneeSlider,       "Knee");
    setupSlider (mixSlider,        "Mix");
    setupSlider (downwardsOutputSlider, "Downwards Output");

    // Setup upwards compressor controls
    setupSlider (upwardsThresholdSlider,  "Upwards Threshold");
    setupSlider (upwardsRatioSlider,      "Upwards Ratio");
    setupSlider (upwardsAttackSlider,     "Upwards Attack");
    setupSlider (upwardsReleaseSlider,    "Upwards Release");
    setupSlider (upwardsKneeSlider,       "Upwards Knee");
    setupSlider (upwardsMixSlider,        "Upwards Mix");
    setupSlider (upwardsOutputSlider,     "Upwards Output");

    // Setup labels for header
    setupLabel(inputGainLabel, "Input");
    setupLabel(outputGainLabel, "Output");
    setupLabel(globalMixLabel, "Global Mix");
    setupLabel(thresholdLabel, "Threshold");
    setupLabel(ratioLabel, "Ratio");
    setupLabel(attackLabel, "Attack");
    setupLabel(releaseLabel, "Release");
    setupLabel(kneeLabel, "Knee");
    setupLabel(mixLabel, "Mix");
    setupLabel(downwardsOutputLabel, "Output");

    // Setup labels for upwards compressor
    setupLabel(upwardsThresholdLabel, "Up Threshold");
    setupLabel(upwardsRatioLabel, "Up Ratio");
    setupLabel(upwardsAttackLabel, "Up Attack");
    setupLabel(upwardsReleaseLabel, "Up Release");
    setupLabel(upwardsKneeLabel, "Up Knee");
    setupLabel(upwardsMixLabel, "Up Mix");
    setupLabel(upwardsOutputLabel, "Up Output");

    // Setup vocal mode button
    vocalModeButton.setButtonText("Vocal Mode");
    vocalModeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    vocalModeButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::skyblue);
    vocalModeButton.onClick = [this]() {
        if (vocalModeButton.getToggleState())
            drumbusModeButton.setToggleState(false, juce::sendNotificationSync);
    };
    this->addAndMakeVisible(vocalModeButton);
    
    // Setup drumbus mode button
    drumbusModeButton.setButtonText("Drumbus Mode");
    drumbusModeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    drumbusModeButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::skyblue);
    drumbusModeButton.onClick = [this]() {
        if (drumbusModeButton.getToggleState())
            vocalModeButton.setToggleState(false, juce::sendNotificationSync);
    };
    this->addAndMakeVisible(drumbusModeButton);

    // Setup upwards first button
    upwardsFirstButton.setButtonText("Upwards First");
    upwardsFirstButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    upwardsFirstButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::skyblue);
    this->addAndMakeVisible(upwardsFirstButton);

    // Add all sliders and labels to the UI
    this->addAndMakeVisible(inputGainSlider);
    this->addAndMakeVisible(outputGainSlider);
    this->addAndMakeVisible(globalMixSlider);
    this->addAndMakeVisible(thresholdSlider);
    this->addAndMakeVisible(ratioSlider);
    this->addAndMakeVisible(attackSlider);
    this->addAndMakeVisible(releaseSlider);
    this->addAndMakeVisible(kneeSlider);
    this->addAndMakeVisible(mixSlider);
    this->addAndMakeVisible(downwardsOutputSlider);
    this->addAndMakeVisible(upwardsThresholdSlider);
    this->addAndMakeVisible(upwardsRatioSlider);
    this->addAndMakeVisible(upwardsAttackSlider);
    this->addAndMakeVisible(upwardsReleaseSlider);
    this->addAndMakeVisible(upwardsKneeSlider);
    this->addAndMakeVisible(upwardsMixSlider);
    this->addAndMakeVisible(upwardsOutputSlider);
    
    this->addAndMakeVisible(inputGainLabel);
    this->addAndMakeVisible(outputGainLabel);
    this->addAndMakeVisible(globalMixLabel);
    this->addAndMakeVisible(thresholdLabel);
    this->addAndMakeVisible(ratioLabel);
    this->addAndMakeVisible(attackLabel);
    this->addAndMakeVisible(releaseLabel);
    this->addAndMakeVisible(kneeLabel);
    this->addAndMakeVisible(mixLabel);
    this->addAndMakeVisible(downwardsOutputLabel);
    this->addAndMakeVisible(upwardsThresholdLabel);
    this->addAndMakeVisible(upwardsRatioLabel);
    this->addAndMakeVisible(upwardsAttackLabel);
    this->addAndMakeVisible(upwardsReleaseLabel);
    this->addAndMakeVisible(upwardsKneeLabel);
    this->addAndMakeVisible(upwardsMixLabel);
    this->addAndMakeVisible(upwardsOutputLabel);

    this->addAndMakeVisible (grMeter);
    this->addAndMakeVisible (inputMeter);
    this->addAndMakeVisible (outputMeter);
    this->addAndMakeVisible (upwardsMeter);
    
    // Setup meter labels
    setupLabel(inputMeterLabel, "INPUT");
    setupLabel(outputMeterLabel, "OUTPUT");
    setupLabel(grMeterLabel, "GR");
    setupLabel(upwardsMeterLabel, "UP GAIN");
    
    // Setup section labels
    setupSectionLabel(downwardsSectionLabel, "DOWNWARDS COMPRESSOR");
    setupSectionLabel(upwardsSectionLabel, "UPWARDS COMPRESSOR");
    
    // Setup bypass buttons with custom logic
    downwardsBypassButton.setButtonText("");
    downwardsBypassButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::skyblue);
    downwardsBypassButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
    this->addAndMakeVisible(downwardsBypassButton);
    
    upwardsBypassButton.setButtonText("");
    upwardsBypassButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::skyblue);
    upwardsBypassButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
    this->addAndMakeVisible(upwardsBypassButton);
    
    // Setup custom bypass value listeners
    downwardsBypassValue.addListener(this);
    upwardsBypassValue.addListener(this);
    
    // Connect bypass values to parameters
    downwardsBypassValue.referTo(apvts.getParameterAsValue("DOWNWARDS_BYPASS"));
    upwardsBypassValue.referTo(apvts.getParameterAsValue("UPWARDS_BYPASS"));
    
    // Initialize bypass button states
    bool downwardsBypassed = downwardsBypassValue.getValue();
    bool upwardsBypassed = upwardsBypassValue.getValue();
    downwardsBypassButton.setToggleState(!downwardsBypassed, juce::dontSendNotification);
    upwardsBypassButton.setToggleState(!upwardsBypassed, juce::dontSendNotification);
    
    this->addAndMakeVisible(inputMeterLabel);
    this->addAndMakeVisible(outputMeterLabel);
    this->addAndMakeVisible(grMeterLabel);
    this->addAndMakeVisible(upwardsMeterLabel);
    this->addAndMakeVisible(downwardsSectionLabel);
    this->addAndMakeVisible(upwardsSectionLabel);

    // Attachments for downwards compressor
    inputGainAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "INPUT_GAIN", inputGainSlider));
    outputGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "OUTPUT_GAIN", outputGainSlider));
    globalMixAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "GLOBAL_MIX", globalMixSlider));
    thresholdAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "THRESHOLD", thresholdSlider));
    ratioAttachment.reset     (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "RATIO", ratioSlider));
    attackAttachment.reset    (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "ATTACK", attackSlider));
    releaseAttachment.reset   (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "RELEASE", releaseSlider));
    kneeAttachment.reset      (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "KNEE", kneeSlider));
    mixAttachment.reset       (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "MIX", mixSlider));
    downwardsOutputAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "DOWNWARDS_OUTPUT", downwardsOutputSlider));
    vocalModeAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (apvts, "VOCAL_MODE", vocalModeButton));
    drumbusModeAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment (apvts, "DRUMBUS_MODE", drumbusModeButton));

    // Attachments for upwards compressor
    upwardsThresholdAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "UPWARDS_THRESHOLD", upwardsThresholdSlider));
    upwardsRatioAttachment.reset     (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "UPWARDS_RATIO", upwardsRatioSlider));
    upwardsAttackAttachment.reset    (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "UPWARDS_ATTACK", upwardsAttackSlider));
    upwardsReleaseAttachment.reset   (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "UPWARDS_RELEASE", upwardsReleaseSlider));
    upwardsKneeAttachment.reset      (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "UPWARDS_KNEE", upwardsKneeSlider));
    upwardsMixAttachment.reset       (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "UPWARDS_MIX", upwardsMixSlider));
    upwardsOutputAttachment.reset    (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, "UPWARDS_OUTPUT", upwardsOutputSlider));
    upwardsFirstAttachment.reset     (new juce::AudioProcessorValueTreeState::ButtonAttachment (apvts, "UPWARDS_FIRST", upwardsFirstButton));

    setResizable (false, false); // Disable resizing
    setSize (800, 850); // Fixed size matching the desired larger layout

    startTimerHz (60);
    
    // Setup bypass button click handlers
    downwardsBypassButton.onClick = [this]() {
        bool currentState = downwardsBypassValue.getValue();
        downwardsBypassValue.setValue(!currentState);
    };
    
    upwardsBypassButton.onClick = [this]() {
        bool currentState = upwardsBypassValue.getValue();
        upwardsBypassValue.setValue(!currentState);
    };
}

CompressorPluginAudioProcessorEditor::~CompressorPluginAudioProcessorEditor()
{
    stopTimer();
    downwardsBypassValue.removeListener(this);
    upwardsBypassValue.removeListener(this);
}

void CompressorPluginAudioProcessorEditor::valueChanged(juce::Value& value)
{
    // Invert the bypass button state - when parameter is false (not bypassed), button should show checkmark
    if (&value == &downwardsBypassValue)
    {
        bool isBypassed = value.getValue();
        downwardsBypassButton.setToggleState(!isBypassed, juce::dontSendNotification);
    }
    else if (&value == &upwardsBypassValue)
    {
        bool isBypassed = value.getValue();
        upwardsBypassButton.setToggleState(!isBypassed, juce::dontSendNotification);
    }
}

void CompressorPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF0E0F12));

    // Panels
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.fillRoundedRectangle (juce::Rectangle<float> (16, 16, (float) getWidth() - 32, (float) getHeight() - 32), 12.0f);
    
    // Header section background
    g.setColour (juce::Colours::white.withAlpha (0.03f));
    g.fillRoundedRectangle (juce::Rectangle<float> (20, 20, (float) getWidth() - 40, 96), 8.0f);
    
    // Footer section background
    g.setColour (juce::Colours::white.withAlpha (0.03f));
    g.fillRoundedRectangle (juce::Rectangle<float> (20, (float) getHeight() - 66, (float) getWidth() - 40, 46), 8.0f);
    
    // Draw separator line between compressor sections - dynamically positioned
    auto area = getLocalBounds().reduced (24);
    area.removeFromTop (120); // Skip header section
    auto left = area.removeFromLeft (area.proportionOfWidth (0.65f));
    left.removeFromTop (10); // Skip top margin
    
    // Calculate section heights to match resized() method
    const int totalHeight = left.getHeight();
    const int downwardsHeight = totalHeight * 0.45f; // 45% for downwards (top)
    const int separatorHeight = totalHeight * 0.05f; // Reduced from 10% to 5% for less space
    const int upwardsHeight = totalHeight * 0.45f; // 45% for upwards (bottom)
    
    // Calculate separator line position based on actual section layout
    const int separatorY = 24 + 120 + 20 + 10 + downwardsHeight + (separatorHeight / 2);
    const int separatorX1 = 24 + 20;
    
    // Calculate where the line should end to avoid bleeding into meters
    // The line should stop well before reaching the input meter area
    const int rightAreaStart = area.getX() + area.proportionOfWidth(0.65f);
    const int separatorX2 = rightAreaStart - 180; // Stop 120 pixels before the right area starts
    
    g.setColour (juce::Colours::white.withAlpha (0.15f));
    g.drawLine ((float) separatorX1, (float) separatorY, (float) separatorX2, (float) separatorY, 1.0f);
}

void CompressorPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (24);

    // Header section - Input/Output controls
    auto headerSection = area.removeFromTop (100);
    
    // Center the header controls
    const int headerKnobW = 90;
    const int headerLabelH = 20;
    const int headerKnobH = 70;
    const int headerSpacing = 30;
    const int totalHeaderWidth = headerKnobW * 3 + headerSpacing * 2;
    const int headerStartX = (area.getWidth() - totalHeaderWidth) / 2;
    
    // Input gain in header
    auto inputArea = juce::Rectangle<int>(headerStartX, headerSection.getY() + 10, headerKnobW, headerSection.getHeight() - 20);
    inputGainLabel.setBounds (inputArea.removeFromTop (headerLabelH));
    inputGainSlider.setBounds (inputArea.removeFromTop (headerKnobH));
    
    // Output gain in header
    auto outputArea = juce::Rectangle<int>(headerStartX + headerKnobW + headerSpacing, headerSection.getY() + 10, headerKnobW, headerSection.getHeight() - 20);
    outputGainLabel.setBounds (outputArea.removeFromTop (headerLabelH));
    outputGainSlider.setBounds (outputArea.removeFromTop (headerKnobH));
    
    // Global mix in header
    auto globalMixArea = juce::Rectangle<int>(headerStartX + (headerKnobW + headerSpacing) * 2, headerSection.getY() + 10, headerKnobW, headerSection.getHeight() - 20);
    globalMixLabel.setBounds (globalMixArea.removeFromTop (headerLabelH));
    globalMixSlider.setBounds (globalMixArea.removeFromTop (headerKnobH));
    
    // Add some spacing after header
    area.removeFromTop (20);

    // Main content area
    auto left = area.removeFromLeft (area.proportionOfWidth (0.65f));
    auto right = area;

    // Add some top margin
    left.removeFromTop (10);

    // Calculate section heights - make sure both sections are visible
    const int totalHeight = left.getHeight();
    const int downwardsHeight = totalHeight * 0.45f; // 45% for downwards (top)
    const int separatorHeight = totalHeight * 0.05f; // Reduced from 10% to 5% for less space
    const int upwardsHeight = totalHeight * 0.45f; // 45% for upwards (bottom)

    // Position section labels first
    downwardsSectionLabel.setBounds (left.getX(), left.getY() - 30, left.getWidth(), 20);
    upwardsSectionLabel.setBounds (left.getX(), left.getY() + downwardsHeight + separatorHeight + 10, left.getWidth(), 20);
    
    // Position bypass buttons beside section labels
    const int bypassButtonSize = 20;
    downwardsBypassButton.setBounds (left.getX() + left.getWidth() - bypassButtonSize - 10, left.getY() - 30, bypassButtonSize, bypassButtonSize);
    upwardsBypassButton.setBounds (left.getX() + left.getWidth() - bypassButtonSize - 10, left.getY() + downwardsHeight + separatorHeight + 10, bypassButtonSize, bypassButtonSize);

    // Calculate responsive knob sizes and spacing based on available width
    const int minKnobW = 65;
    const int maxKnobW = 90;
    const int availableWidth = left.getWidth() - 40; // Leave some margin
    const int knobW = juce::jlimit(minKnobW, maxKnobW, availableWidth / 4 - 8); // 4 knobs per row with spacing
    const int knobH = juce::jlimit(60, 80, static_cast<int>(knobW * 0.8f)); // Proportional height
    const int labelH = 18;
    const int rowSpacing = juce::jlimit(10, 20, static_cast<int>(totalHeight * 0.02f)); // Proportional row spacing
    const int knobSpacing = juce::jlimit(10, 20, static_cast<int>(knobW * 0.15f)); // Proportional knob spacing
    
    // Calculate total width needed for knobs (4 knobs per row)
    const int totalKnobWidth = knobW * 4 + knobSpacing * 3;
    const int startX = left.getX() + 20; // Align more to the left with fixed margin
    
    // Downwards compressor controls - positioned in the top section
    int currentX = startX;
    int currentY = left.getY() + 10;
    
    // Top row - Threshold, Ratio, Knee, Output
    // Threshold
    auto thresholdArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    thresholdLabel.setBounds (thresholdArea.removeFromTop (labelH));
    thresholdSlider.setBounds (thresholdArea);
    currentX += knobW + knobSpacing;
    
    // Ratio
    auto ratioArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    ratioLabel.setBounds (ratioArea.removeFromTop (labelH));
    ratioSlider.setBounds (ratioArea);
    currentX += knobW + knobSpacing;
    
    // Knee
    auto kneeArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    kneeLabel.setBounds (kneeArea.removeFromTop (labelH));
    kneeSlider.setBounds (kneeArea);
    currentX += knobW + knobSpacing;
    
    // Output
    auto downwardsOutputArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    downwardsOutputLabel.setBounds (downwardsOutputArea.removeFromTop (labelH));
    downwardsOutputSlider.setBounds (downwardsOutputArea);
    
    // Bottom row - Attack, Release, Mix (reset X position for consistent alignment)
    currentX = startX;
    currentY += knobH + labelH + rowSpacing;
    
    // Attack
    auto attackArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    attackLabel.setBounds (attackArea.removeFromTop (labelH));
    attackSlider.setBounds (attackArea);
    currentX += knobW + knobSpacing;
    
    // Release
    auto releaseArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    releaseLabel.setBounds (releaseArea.removeFromTop (labelH));
    releaseSlider.setBounds (releaseArea);
    currentX += knobW + knobSpacing;
    
    // Mix
    auto mixArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    mixLabel.setBounds (mixArea.removeFromTop (labelH));
    mixSlider.setBounds (mixArea);

    // Upwards compressor section (BOTTOM) - Position controls directly with consistent alignment
    currentX = startX;
    currentY = left.getY() + downwardsHeight + separatorHeight + 40;
    
    // Top row - Up Threshold, Up Ratio, Up Knee, Up Output
    // Up Threshold
    auto upThresholdArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    upwardsThresholdLabel.setBounds (upThresholdArea.removeFromTop (labelH));
    upwardsThresholdSlider.setBounds (upThresholdArea);
    currentX += knobW + knobSpacing;
    
    // Up Ratio
    auto upRatioArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    upwardsRatioLabel.setBounds (upRatioArea.removeFromTop (labelH));
    upwardsRatioSlider.setBounds (upRatioArea);
    currentX += knobW + knobSpacing;
    
    // Up Knee
    auto upKneeArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    upwardsKneeLabel.setBounds (upKneeArea.removeFromTop (labelH));
    upwardsKneeSlider.setBounds (upKneeArea);
    currentX += knobW + knobSpacing;
    
    // Up Output
    auto upOutputArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    upwardsOutputLabel.setBounds (upOutputArea.removeFromTop (labelH));
    upwardsOutputSlider.setBounds (upOutputArea);
    
    // Bottom row - Up Attack, Up Release, Up Mix (reset X position for consistent alignment)
    currentX = startX;
    currentY += knobH + labelH + rowSpacing;
    
    // Up Attack
    auto upAttackArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    upwardsAttackLabel.setBounds (upAttackArea.removeFromTop (labelH));
    upwardsAttackSlider.setBounds (upAttackArea);
    currentX += knobW + knobSpacing;
    
    // Up Release
    auto upReleaseArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    upwardsReleaseLabel.setBounds (upReleaseArea.removeFromTop (labelH));
    upwardsReleaseSlider.setBounds (upReleaseArea);
    currentX += knobW + knobSpacing;
    
    // Up Mix
    auto upMixArea = juce::Rectangle<int>(currentX, currentY, knobW, knobH + labelH);
    upwardsMixLabel.setBounds (upMixArea.removeFromTop (labelH));
    upwardsMixSlider.setBounds (upMixArea);

    // Right side meters - properly sized and spaced with responsive sizing
    auto meterArea = right.reduced (8);
    const int minMeterW = 60;
    const int maxMeterW = 90;
    const int meterW = juce::jlimit(minMeterW, maxMeterW, meterArea.getWidth() / 4 - 5);
    const int meterSpacing = juce::jlimit(5, 12, static_cast<int>(meterW * 0.15f)); // Proportional meter spacing
    
    // Position meters higher, closer to header and away from footer
    const int meterStartX = meterArea.getX() + (meterArea.getWidth() - (meterW * 4 + meterSpacing * 3)) / 2;
    const int meterY = meterArea.getY() + 10; // Start closer to header
    const int meterH = meterArea.getHeight() - 140; // Leave space for footer
    
    // Input meter
    inputMeter.setBounds (meterStartX, meterY, meterW, meterH);
    inputMeterLabel.setBounds (meterStartX, meterY - 20, meterW, 20);
    
    // Output meter
    outputMeter.setBounds (meterStartX + meterW + meterSpacing, meterY, meterW, meterH);
    outputMeterLabel.setBounds (meterStartX + meterW + meterSpacing, meterY - 20, meterW, 20);
    
    // GR meter
    grMeter.setBounds (meterStartX + (meterW + meterSpacing) * 2, meterY, meterW, meterH);
    grMeterLabel.setBounds (meterStartX + (meterW + meterSpacing) * 2, meterY - 20, meterW, 20);
    
    // Upwards gain meter
    upwardsMeter.setBounds (meterStartX + (meterW + meterSpacing) * 3, meterY, meterW, meterH);
    upwardsMeterLabel.setBounds (meterStartX + (meterW + meterSpacing) * 3, meterY - 20, meterW, 20);

    // Footer section - Mode buttons centered with responsive sizing
    auto footerSection = area.removeFromBottom (120); // Increased from 100 to 120 for more space
    
    // Center the mode buttons in the footer with responsive sizing
    const int minButtonW = 80;
    const int maxButtonW = 120;
    const int buttonW = juce::jlimit(minButtonW, maxButtonW, (getWidth() - 100) / 3); // Use full plugin width like header
    const int buttonH = 25;
    const int buttonSpacing = juce::jlimit(15, 25, static_cast<int>(buttonW * 0.2f)); // Proportional button spacing
    const int totalButtonWidth = buttonW * 3 + buttonSpacing * 2;
    const int footerStartX = (getWidth() - totalButtonWidth) / 2; // Center relative to full plugin width like header
    
    // Position buttons in the center of the footer area, properly within the grey footer
    const int footerCenterY = footerSection.getY() + (footerSection.getHeight() - buttonH) / 2;
    
    upwardsFirstButton.setBounds (footerStartX, footerCenterY, buttonW, buttonH);
    vocalModeButton.setBounds (footerStartX + buttonW + buttonSpacing, footerCenterY, buttonW, buttonH);
    drumbusModeButton.setBounds (footerStartX + (buttonW + buttonSpacing) * 2, footerCenterY, buttonW, buttonH);
}

void CompressorPluginAudioProcessorEditor::timerCallback()
{
    // Show 0 on meters when compressors are bypassed
    if (processor.isDownwardsBypassed())
        grMeter.setValue (0.0f);
    else
        grMeter.setValue (processor.getGainReduction());
    
    // Smooth input and output meters for easier reading
    const float inputLevel = processor.getInputLevel();
    const float outputLevel = processor.getOutputLevel();
    
    // Smoothing coefficient (0.95 = slower, 0.85 = faster)
    const float smoothingCoeff = 0.82f;
    
    smoothedInputLevel = smoothedInputLevel * smoothingCoeff + inputLevel * (1.0f - smoothingCoeff);
    smoothedOutputLevel = smoothedOutputLevel * smoothingCoeff + outputLevel * (1.0f - smoothingCoeff);
    
    inputMeter.setInputValue (smoothedInputLevel);
    outputMeter.setOutputValue (smoothedOutputLevel);
    
    if (processor.isUpwardsBypassed())
        upwardsMeter.setUpwardsGainValue (0.0f);
    else
        upwardsMeter.setUpwardsGainValue (processor.getUpwardsGain());
}

void CompressorPluginAudioProcessorEditor::setupSlider(juce::Slider& s, const juce::String& name, bool isDial)
{
    s.setName (name);
    if (name == "Mix" || name == "Global Mix" || name == "Up Mix" || name == "Upwards Mix") s.setTextValueSuffix (" %");
    else if (name == "Ratio" || name == "Upwards Ratio") s.setTextValueSuffix (":1");
    else if (name == "Attack" || name == "Release" || name == "Up Attack" || name == "Up Release" || name == "Upwards Attack" || name == "Upwards Release") s.setTextValueSuffix (" ms");
    else s.setTextValueSuffix (" dB");

    s.setSliderStyle (isDial ? juce::Slider::RotaryHorizontalVerticalDrag
                             : juce::Slider::LinearHorizontal);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible (s);
}

void CompressorPluginAudioProcessorEditor::setupLabel(juce::Label& l, const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
    l.setColour(juce::Label::textColourId, juce::Colours::white);
    l.setFont(juce::Font(12.0f, juce::Font::bold));
    l.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(l);
}

void CompressorPluginAudioProcessorEditor::setupSectionLabel(juce::Label& l, const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
    l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    l.setFont(juce::Font(11.0f, juce::Font::bold));
    l.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(l);
}


