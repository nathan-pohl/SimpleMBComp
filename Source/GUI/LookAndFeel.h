/*
  ==============================================================================

    LookAndFeel.h
    Created: 12 Feb 2023 3:55:07pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct LookAndFeel : juce::LookAndFeel_V4 {
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;

    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
};