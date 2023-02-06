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
//const juce::String THRESHOLD_NAME = "Threshold";
//const juce::String ATTACK_NAME = "Attack";
//const juce::String RELEASE_NAME = "Release";
//const juce::String RATIO_NAME = "Ratio";
//const juce::String BYPASS_NAME = "Bypassed";

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

const bool APVTS_BOOL_DEFAULT = false;

const float LOW_MID_MIN_FREQ = 20.f;
const float LOW_MID_MAX_FREQ = 999.f;
const float MID_HIGH_MIN_FREQ = 1000.f;
const float MID_HIGH_MAX_FREQ = 20000.f; // 20 kHz
const float LOW_MID_CROSSOVER_DEFAULT_FREQUENCY = 400.f;
const float MID_HIGH_CROSSOVER_DEFAULT_FREQUENCY = 2000.f;

const float GAIN_MIN = -24.f;
const float GAIN_MAX = 24.f;
const float GAIN_INTERVAL = 0.5f;
const float GAIN_DEFAULT = 0.f;

const float SLIDER_MIN_RANGE = 0.f;
const double SLIDER_MIN_RANGE_DOUBLE = 0.0;
const float SLIDER_MAX_RANGE = 1.f;
const double SLIDER_MAX_RANGE_DOUBLE = 1.0;

//==============================================================================
// Units
const juce::String HZ = "Hz";
const juce::String KILO_HZ = "KHz";
const juce::String DB = "dB";
const juce::String DB_PER_OCT = "db/Oct";

//==============================================================================
// GUI Values
const float UI_BOUNDS_HALF = 0.5f;

const float FLEX_DEFAULT = 1.f;
const int FLEX_SPACER = 4;
const int FLEX_END_CAP = 6;

const int CONTROL_BAR_HEIGHT = 32;
const int BAND_CONTROLS_HEIGHT = 135;
const int ANALYZER_HEIGHT = 225;

const int DEFAULT_PADDING = 5;
const int SLIDER_X_PADDING = 2;

const int NUMBER_OF_LINES_TEXT = 1;

const int BORDER_THICKNESS = 3;
const float ELLIPSE_DEFAULT_THICKNESS = 1.f;
const float ELLIPSE_THICKER_OUTLINE = 2.f;
const float ROUNDED_RECTANGLE_THICKNESS = 2.f;
const float PATH_STROKE_THICKNESS = 1.f;

const int TEXT_BOUNDING_BOX_ADD_WIDTH = 4;
const int TEXT_BOUNDING_BOX_ADD_HEIGHT = 2;