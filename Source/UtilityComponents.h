/*
  ==============================================================================

    UtilityComponents.h
    Created: 12 Feb 2023 4:03:58pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
struct Placeholder : juce::Component {
    Placeholder();

    void paint(juce::Graphics& g) override;

    juce::Colour customColor;
};

//==============================================================================
struct RotarySlider : juce::Slider {
    RotarySlider();
};