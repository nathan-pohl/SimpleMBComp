/*
  ==============================================================================

    Utilities.cpp
    Created: 12 Feb 2023 4:06:56pm
    Author:  Nate

  ==============================================================================
*/

#include "Utilities.h"
#include "../Constants.h"

template<typename T>
bool truncateKiloValue(T& value) {
    if (value > static_cast<T>(999)) {
        value /= static_cast<T>(1000);
        return true;
    }
    return false;
}

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix) {
    juce::String str;
    auto val = getLow ? param.getNormalisableRange().start : param.getNormalisableRange().end;

    bool useK = truncateKiloValue(val);
    str << val;
    if (useK) {
        str << "K";
    }
    str << suffix;

    return str;
}

void drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds) {
    using namespace juce;
    // setting a border color of blue vioet, then black fill
    g.setColour(Colours::blueviolet);
    g.fillAll();

    auto localBounds = bounds;

    bounds.reduce(BORDER_THICKNESS, BORDER_THICKNESS);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);

    g.drawRect(localBounds);
}