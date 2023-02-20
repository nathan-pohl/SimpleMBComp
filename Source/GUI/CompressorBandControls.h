/*
  ==============================================================================

    CompressorBandControls.h
    Created: 12 Feb 2023 4:11:32pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"

struct CompressorBandControls : juce::Component, juce::Button::Listener {
    CompressorBandControls(juce::AudioProcessorValueTreeState& apvts);
    ~CompressorBandControls() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void toggleAllBands(bool shouldBeBypassed);
private:
    juce::AudioProcessorValueTreeState& apvts;

    RotarySliderWithLabels attackSlider, releaseSlider, thresholdSlider;
    RatioSlider ratioSlider;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attackSliderAttachment,
        releaseSliderAttachment,
        thresholdSliderAttachment,
        ratioSliderAttachment;

    using BtnAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<BtnAttachment> bypassButtonAttachment,
        soloButtonAttachment,
        muteButtonAttachment;

    juce::ToggleButton bypassButton, soloButton, muteButton, lowBand, midBand, highBand;
    juce::Component::SafePointer<CompressorBandControls> safePtr{ this };

    juce::ToggleButton* activeBand = &lowBand;

    void updateAttachments();
    void updateSliderEnablements();
    void updateSoloMuteBypassToggleStates(juce::Button& button);
    void updateActiveBandFillColors(juce::Button& clickedButton);
    void resetActiveBandColors();
    static void refreshBandButtonColors(juce::Button& band, juce::Button& colorSource);
    void updateBandSelectButtonStates();
};