/*
  ==============================================================================

    ControlBar.cpp
    Created: 20 Feb 2023 10:53:41am
    Author:  Nate

  ==============================================================================
*/

#include "ControlBar.h"
#include "../Constants.h"

ControlBar::ControlBar() {
    analyzerButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(analyzerButton);
    addAndMakeVisible(globalBypassButton);
}

void ControlBar::resized() {
    auto bounds = getLocalBounds();
    analyzerButton.setBounds(bounds.removeFromLeft(ANALYZER_BUTTON_SPACING).withTrimmedTop(ANALYZER_BUTTON_PADDING).withTrimmedBottom(ANALYZER_BUTTON_PADDING));
    globalBypassButton.setBounds(bounds.removeFromRight(POWER_BUTTON_SPACING).withTrimmedTop(POWER_BUTTON_PADDING).withTrimmedBottom(POWER_BUTTON_PADDING));
}