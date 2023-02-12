/*
  ==============================================================================

    CustomButtons.h
    Created: 12 Feb 2023 4:00:41pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

// We have two different kinds of buttons we want to use, but want to use the same LookAndFeel functions between them.
// Using these inherited classes to make it easier to determine between the two
struct PowerButton : juce::ToggleButton { };
struct AnalyzerButton : juce::ToggleButton {
    void resized() override;

    juce::Path randomPath;
};
