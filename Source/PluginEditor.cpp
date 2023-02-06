/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
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
}

//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g) {
    using namespace juce;
    // 7 o'clock is where slider draws value of zero, 5 o'clock is where slider draws value of one
    float startAngle = degreesToRadians(180.f + 45.f); // 7 o'clock basically
    float endAngle = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi; // 5 o'clock basically - needs a full rotation to put it at the right angle though, otherwise slider will go the wrong way

    Range<double> range = getRange();
    Rectangle<int> sliderBounds = getSliderBounds();
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
    int size = juce::jmin(bounds.getWidth(), bounds.getHeight()); // Make our sliders square by getting the minimum size from the width and height
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
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
        if (val > 999.f) {
            val /= 1000.f;
            addK = true; // Use KHz for units
        }
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

//==============================================================================
Placeholder::Placeholder() {
    // Use a random color for now
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
};

//==============================================================================
GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts) {
    using namespace Params;
    const auto& params = GetParams();

    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto& name, auto& slider) {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(inGainSliderAttachment, Names::GainIn, inGainSlider);
    makeAttachmentHelper(lowMidXoverSliderAttachment, Names::LowMidCrossoverFreq, lowMidXoverSlider);
    makeAttachmentHelper(midHighXoverSliderAttachment, Names::MidHighCrossoverFreq, midHighXoverSlider);
    makeAttachmentHelper(outGainSliderAttachment, Names::GainOut, outGainSlider);

    addAndMakeVisible(inGainSlider);
    addAndMakeVisible(lowMidXoverSlider);
    addAndMakeVisible(midHighXoverSlider);
    addAndMakeVisible(outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g) {
    using namespace juce;
    auto bounds = getLocalBounds();
    // setting a border color of blue vioet, then black fill
    g.setColour(Colours::blueviolet);
    g.fillAll();

    auto localBounds = bounds;

    bounds.reduce(BORDER_THICKNESS, BORDER_THICKNESS);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);

    g.drawRect(localBounds);
}

void GlobalControls::resized() {
    auto bounds = getLocalBounds();
    using namespace juce;

    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    flexBox.items.add(FlexItem(inGainSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(FlexItem(lowMidXoverSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(FlexItem(midHighXoverSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(FlexItem(outGainSlider).withFlex(FLEX_DEFAULT));

    flexBox.performLayout(bounds);
}

//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (600, 500);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
}

//==============================================================================
void SimpleMBCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleMBCompAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    controlBar.setBounds(bounds.removeFromTop(CONTROL_BAR_HEIGHT));
    bandControls.setBounds(bounds.removeFromBottom(BAND_CONTROLS_HEIGHT));
    analyzer.setBounds(bounds.removeFromTop(ANALYZER_HEIGHT));
    globalControls.setBounds(bounds);
}
