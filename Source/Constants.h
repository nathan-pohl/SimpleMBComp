/*
  ==============================================================================

    Constants.h
    Created: 29 Jan 2023 4:43:00pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

// APVTS Names
const juce::String THRESHOLD_NAME = "Threshold";
const juce::String ATTACK_NAME = "Attack";
const juce::String RELEASE_NAME = "Release";
const juce::String RATIO_NAME = "Ratio";

//==============================================================================
// Ranges
const float THRESHOLD_MIN_VAL = -60.f;
const float THRESHOLD_MAX_VAL = 12.f;
const float THRESHOLD_DEFAULT = 1.f;

const float ATTACK_RELEASE_MIN_VAL = 5.f;
const float ATTACK_RELEASE_MAX_VAL = 500.f;
const float ATTACK_DEFAULT = 50.f;
const float RELEASE_DEFAULT = 250.f;

const float DEFAULT_INTERVAL = 1.f;
const float DEFAULT_SKEW_FACTOR = 1.f;

const auto RATIO_CHOICES = std::vector<double>{ 1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100 };
const float RATIO_DEFAULT = 3.f;