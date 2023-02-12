/*
  ==============================================================================

    RotarySliderWithLabels.cpp
    Created: 12 Feb 2023 3:58:21pm
    Author:  Nate

  ==============================================================================
*/

#include "RotarySliderWithLabels.h"
#include "Utilities.cpp"


void RotarySliderWithLabels::paint(juce::Graphics& g) {
    using namespace juce;
    // 7 o'clock is where slider draws value of zero, 5 o'clock is where slider draws value of one
    float startAngle = degreesToRadians(180.f + 45.f); // 7 o'clock basically
    float endAngle = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi; // 5 o'clock basically - needs a full rotation to put it at the right angle though, otherwise slider will go the wrong way

    Range<double> range = getRange();
    Rectangle<int> sliderBounds = getSliderBounds();

    auto bounds = getLocalBounds();
    g.setColour(Colours::blueviolet);
    g.drawFittedText(getName(), bounds.removeFromTop(getTextHeight() + TEXT_BOUNDING_BOX_ADD_HEIGHT), Justification::centredBottom, 1);
    // Draw bounds of the slider
    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(Colours::yellow);
    //g.drawRect(sliderBounds);
    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), SLIDER_MIN_RANGE_DOUBLE, SLIDER_MAX_RANGE_DOUBLE),  // normalize values in the range
        startAngle,
        endAngle,
        *this);

    Point<float> center = sliderBounds.toFloat().getCentre();
    float radius = sliderBounds.getWidth() * UI_BOUNDS_HALF;
    g.setColour(Colour(0u, 172u, 1u));
    g.setFont(getTextHeight());

    int numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i) {
        float pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);

        // This logic determines where to place the labels for the bounding points for each slider (e.g. 20 Hz to 20KHz)
        float angle = jmap(pos, 0.f, 1.f, startAngle, endAngle);
        Point<float> c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, angle);
        Rectangle<float> r;
        String str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, NUMBER_OF_LINES_TEXT);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.removeFromTop(getTextHeight() * TEXT_HEIGHT_MULTIPLIER);
    int size = juce::jmin(bounds.getWidth(), bounds.getHeight()); // Make our sliders square by getting the minimum size from the width and height
    size -= getTextHeight() * TEXT_HEIGHT_MULTIPLIER;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    //r.setY(2);
    r.setY(bounds.getY());
    return r;
}

//int RotarySliderWithLabels::getTextHeight() const { return 14; }
juce::String RotarySliderWithLabels::getDisplayString() const {
    // If the parameter is a choice parameter, show the choice name e.g. 12 db/Oct
    if (juce::AudioParameterChoice* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        return choiceParam->getCurrentChoiceName();
    }

    juce::String str;
    bool addK = false; // For Kilohertz

    // Only Float parameters are supported here
    if (juce::AudioParameterFloat* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
        float val = getValue();
        //if (val > 999.f) {
        //    val /= 1000.f;
        //    addK = true; // Use KHz for units
        //}
        addK = truncateKiloValue(val);
        str = juce::String(val, (addK ? 2 : 0)); // If we are using KHz, only use 2 decimal places
        if (suffix.isNotEmpty()) {
            str << " ";
            if (addK) {
                str << "K";
            }
            str << suffix;
        }

        return str;
    }
    else {
        jassertfalse;
    }
}

void RotarySliderWithLabels::changeParam(juce::RangedAudioParameter* p) {
    param = p;
    repaint();
}

//==============================================================================
juce::String RatioSlider::getDisplayString() const {
    auto choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param);
    jassert(choiceParam != nullptr);

    auto currentChoice = choiceParam->getCurrentChoiceName();
    if (currentChoice.contains(".0")) {
        currentChoice = currentChoice.substring(0, currentChoice.indexOf("."));
    }
    currentChoice << ":1";
    return currentChoice;
}
