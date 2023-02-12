/*
  ==============================================================================

    RotarySliderWithLabels.h
    Created: 12 Feb 2023 3:58:21pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../Constants.h"

struct RotarySliderWithLabels : juce::Slider {
    RotarySliderWithLabels(juce::RangedAudioParameter* param,
        const juce::String suffixStr,
        const juce::String& title) :
        juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(param),
        suffix(suffixStr)
    {
        setName(title);
    }

    struct LabelPos {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    virtual juce::String getDisplayString() const;
    void changeParam(juce::RangedAudioParameter* p);

protected:
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

struct RatioSlider : RotarySliderWithLabels {
    RatioSlider(juce::RangedAudioParameter* param, const juce::String suffixStr) : RotarySliderWithLabels(param, suffixStr, RATIO_LABEL) {}

    juce::String getDisplayString() const override;
};