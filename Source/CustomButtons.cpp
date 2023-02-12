/*
  ==============================================================================

    CustomButtons.cpp
    Created: 12 Feb 2023 4:00:41pm
    Author:  Nate

  ==============================================================================
*/

#include "CustomButtons.h"

void AnalyzerButton::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    juce::Rectangle<int> insetRect = bounds.reduced(4);

    randomPath.clear();
    juce::Random rand;

    int pathBaseHeight = insetRect.getY() + insetRect.getHeight();
    // Draw a random jagged line to indicate that this is the button for the spectrum analyzer
    randomPath.startNewSubPath(insetRect.getX(), pathBaseHeight * rand.nextFloat());
    for (int x = insetRect.getX() + 1; x < insetRect.getRight(); x += 2) {
        randomPath.lineTo(x, pathBaseHeight * rand.nextFloat());
    }
}