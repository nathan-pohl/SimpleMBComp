/*
  ==============================================================================

    ControlBar.h
    Created: 20 Feb 2023 10:53:41am
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "CustomButtons.h"


struct ControlBar : juce::Component {
    ControlBar();
    void resized() override;
    AnalyzerButton analyzerButton;
    PowerButton globalBypassButton;
};
