/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 12 Feb 2023 3:55:07pm
    Author:  Nate

  ==============================================================================
*/

#include "LookAndFeel.h"
#include "../Constants.h"
#include "RotarySliderWithLabels.h"
#include "CustomButtons.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) {
    using namespace juce;
    bool enabled = slider.isEnabled();

    // background of the Slider
    Rectangle<float> bounds = Rectangle<float>(x, y, width, height);
    g.setColour(enabled ? Colour(97u, 18u, 167u) : Colours::darkgrey); // Purple background of ellipse, dark grey if slider is diabled
    g.fillEllipse(bounds);
    g.setColour(enabled ? Colour(255u, 154u, 1u) : Colours::grey); // Orange border of ellipse, grey if slider is disabled
    g.drawEllipse(bounds, ELLIPSE_DEFAULT_THICKNESS);

    if (RotarySliderWithLabels* rswl = dynamic_cast<RotarySliderWithLabels*> (&slider)) {
        jassert(rotaryStartAngle < rotaryEndAngle); // Check to make sure the angles are set right

        // Draw the rectangle that forms the pointer of the rotary dial // TODO it is not drawing properly
        Point<float> center = bounds.getCentre();
        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX() - SLIDER_X_PADDING); // Two pixels left of center
        r.setRight(center.getX() + SLIDER_X_PADDING); // Two pixels to right of center
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);
        p.addRoundedRectangle(r, ROUNDED_RECTANGLE_THICKNESS);

        float sliderAngleRadians = jmap(sliderPosProportional, SLIDER_MIN_RANGE, SLIDER_MAX_RANGE, rotaryStartAngle, rotaryEndAngle); // Map the slider angle to be between the bounding angles
        p.applyTransform(AffineTransform().rotated(sliderAngleRadians, center.getX(), center.getY()));
        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        juce::String text = rswl->getDisplayString();
        int strWidth = g.getCurrentFont().getStringWidth(text);

        // Draw bounding box for text
        r.setSize(strWidth + TEXT_BOUNDING_BOX_ADD_WIDTH, rswl->getTextHeight() + TEXT_BOUNDING_BOX_ADD_HEIGHT);
        r.setCentre(center);
        g.setColour(enabled ? Colours::black : Colours::darkgrey); // Text background, use darkgrey if slider is disabled
        g.fillRect(r);

        g.setColour(enabled ? Colours::white : Colours::lightgrey); // Text color, use light grey if slider is disabled
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, NUMBER_OF_LINES_TEXT);
    }
}

void LookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& toggleButton, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    using namespace juce;

    if (PowerButton* pb = dynamic_cast<PowerButton*>(&toggleButton)) {
        Path powerButton;
        Rectangle<int> bounds = toggleButton.getLocalBounds();
        int size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
        Rectangle<float> rect = bounds.withSizeKeepingCentre(size, size).toFloat();

        float angle = 30.f;
        size -= 6;

        // These lines draw on the button to create the typical power button symbol. The arc creates that incomplete circle within the button, and the subPath and lineTo draw the line from top to center 
        powerButton.addCentredArc(rect.getCentreX(), rect.getCentreY(), size * 0.5, size * 0.5, 0.f, degreesToRadians(angle), degreesToRadians(360.f - angle), true);
        powerButton.startNewSubPath(rect.getCentreX(), rect.getY());
        powerButton.lineTo(rect.getCentre());

        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);

        // if button is on, then band is bypassed and we should use grey, otherwise, use green to indicate the band is engaged
        juce::Colour color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
        g.setColour(color);
        g.strokePath(powerButton, pst);
        // One thing that could be added to the button is making sure the toggle only happens when clicked inside where the button is actually drawn, 
        // rather than it's bounding box which is much bigger than the ellipse drawn here
        g.drawEllipse(rect, ELLIPSE_THICKER_OUTLINE);
    }
    else if (AnalyzerButton* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton)) {
        // Analyzer button works opposite to power button so if button is off, then the analyzer is on and we should use green
        Colour color = !toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
        g.setColour(color);

        Rectangle<int> bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);
        g.strokePath(analyzerButton->randomPath, PathStrokeType(PATH_STROKE_THICKNESS));
    }
    else {
        // Draw a rounded rectangle with component name in middle
        auto bounds = toggleButton.getLocalBounds().reduced(TOGGLE_BUTTON_PADDING);
        auto buttonIsOn = toggleButton.getToggleState();
        const int cornerSize = 4;
        g.setColour(buttonIsOn ?
                    toggleButton.findColour(TextButton::ColourIds::buttonOnColourId) :
                    toggleButton.findColour(TextButton::ColourIds::buttonColourId));
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize);

        g.setColour(buttonIsOn ? juce::Colours::black : juce::Colours::white);
        g.drawRoundedRectangle(bounds.toFloat(), cornerSize, PATH_STROKE_THICKNESS);
        g.drawFittedText(toggleButton.getName(), bounds, juce::Justification::centred, 1);
    }
}