/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


//==============================================================================
struct LookAndFeel : juce::LookAndFeel_V4 {
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;

    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
};

//==============================================================================
struct RotarySliderWithLabels : juce::Slider {
    RotarySliderWithLabels(juce::RangedAudioParameter* param, 
                           const juce::String suffixStr, 
                           const juce::String& title) : 
        juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(param),
        suffix(suffixStr)
    {
        setName(title);
    }

    struct LabelPos {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    virtual juce::String getDisplayString() const;
    void changeParam(juce::RangedAudioParameter* p);

protected:
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

struct RatioSlider : RotarySliderWithLabels {
    RatioSlider(juce::RangedAudioParameter* param, const juce::String suffixStr) : RotarySliderWithLabels(param, suffixStr, RATIO_LABEL) {}

    juce::String getDisplayString() const override;
};

//==============================================================================
// We have two different kinds of buttons we want to use, but want to use the same LookAndFeel functions between them.
// Using these inherited classes to make it easier to determine between the two
struct PowerButton : juce::ToggleButton { };
struct AnalyzerButton : juce::ToggleButton {
    void resized() override {
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

    juce::Path randomPath;
};

//==============================================================================
struct Placeholder : juce::Component {
    Placeholder();

    void paint(juce::Graphics& g) override {
        g.fillAll(customColor);
    }

    juce::Colour customColor;
};

//==============================================================================
struct RotarySlider : juce::Slider {
    RotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox) {

    }
};

//==============================================================================
template<typename Attachment, typename APVTS, typename Params, typename ParamName, typename SliderType>
void makeAttachment(std::unique_ptr<Attachment>& attachment, APVTS& apvts, const Params& params, const ParamName& name, SliderType& slider) {
    attachment = std::make_unique<Attachment>(apvts, params.at(name), slider);
}

template<typename APVTS, typename Params, typename Name>
juce::RangedAudioParameter& getParam(APVTS& apvts, const Params& params, const Name& name) {
    auto param = apvts.getParameter(params.at(name));
    jassert(param != nullptr);

    return *param;
}

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix);

template<typename Labels, typename ParamType, typename SuffixType>
void addLabelPairs(Labels& labels, const ParamType& param, const SuffixType& suffix) {
    labels.clear();
    labels.add({ 0.f, getValString(param, true, suffix)}); // Minimum value at 0%
    labels.add({ 1.f, getValString(param, false, suffix)}); // Maximum value at 100%
}

//==============================================================================
struct CompressorBandControls : juce::Component {
    CompressorBandControls(juce::AudioProcessorValueTreeState& apvts);

    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    juce::AudioProcessorValueTreeState& apvts;

    RotarySliderWithLabels attackSlider, releaseSlider, thresholdSlider;
    RatioSlider ratioSlider;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attackSliderAttachment,
                                releaseSliderAttachment,
                                thresholdSliderAttachment,
                                ratioSliderAttachment;

    using BtnAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<BtnAttachment> bypassButtonAttachment,
                                soloButtonAttachment,
                                muteButtonAttachment;

    juce::ToggleButton bypassButton, soloButton, muteButton, lowBand, midBand, highBand;
    juce::Component::SafePointer<CompressorBandControls> safePtr{ this };

    void updateAttachments();
};

//==============================================================================
struct GlobalControls : juce::Component {
    GlobalControls(juce::AudioProcessorValueTreeState& apvts);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    using RSWL = RotarySliderWithLabels;
    std::unique_ptr<RSWL> inGainSlider, lowMidXoverSlider, midHighXoverSlider, outGainSlider;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> lowMidXoverSliderAttachment,
                                midHighXoverSliderAttachment,
                                inGainSliderAttachment,
                                outGainSliderAttachment;
};
/**
*/
class SimpleMBCompAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor&);
    ~SimpleMBCompAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LookAndFeel lookAndFeel;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleMBCompAudioProcessor& audioProcessor;

    Placeholder controlBar, analyzer;
    GlobalControls globalControls { audioProcessor.apvts };
    CompressorBandControls bandControls { audioProcessor.apvts };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessorEditor)
};
