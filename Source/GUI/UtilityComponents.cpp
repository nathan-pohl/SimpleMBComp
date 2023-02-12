/*
  ==============================================================================

    UtilityComponents.cpp
    Created: 12 Feb 2023 4:03:58pm
    Author:  Nate

  ==============================================================================
*/

#include "UtilityComponents.h"

Placeholder::Placeholder() {
    // Use a random color for now
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
};

void Placeholder::paint(juce::Graphics& g) {
    g.fillAll(customColor);
}

//==============================================================================
RotarySlider::RotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox) {

}