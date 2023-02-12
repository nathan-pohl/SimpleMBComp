/*
  ==============================================================================

    GlobalControls.cpp
    Created: 12 Feb 2023 4:11:46pm
    Author:  Nate

  ==============================================================================
*/

#include "GlobalControls.h"
#include "Params.h"
#include "Constants.h"
#include "Utilities.h"

GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts) {
    using namespace Params;
    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts](const auto& name) -> auto& {
        return getParam(apvts, params, name);
    };

    auto& gainInParam = getParamHelper(Names::GainIn);
    auto& lowMidCrossoverParam = getParamHelper(Names::LowMidCrossoverFreq);
    auto& midHighCrossoverParam = getParamHelper(Names::MidHighCrossoverFreq);
    auto& gainOutParam = getParamHelper(Names::GainOut);

    inGainSlider = std::make_unique<RSWL>(&gainInParam, DB, GAIN_IN_LABEL);
    lowMidXoverSlider = std::make_unique<RSWL>(&lowMidCrossoverParam, HZ, LOW_MID_XOVER_LABEL);
    midHighXoverSlider = std::make_unique<RSWL>(&midHighCrossoverParam, HZ, MID_HIGH_XOVER_LABEL);
    outGainSlider = std::make_unique<RSWL>(&gainOutParam, DB, GAIN_OUT_LABEL);


    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto& name, auto& slider) {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(inGainSliderAttachment, Names::GainIn, *inGainSlider);
    makeAttachmentHelper(lowMidXoverSliderAttachment, Names::LowMidCrossoverFreq, *lowMidXoverSlider);
    makeAttachmentHelper(midHighXoverSliderAttachment, Names::MidHighCrossoverFreq, *midHighXoverSlider);
    makeAttachmentHelper(outGainSliderAttachment, Names::GainOut, *outGainSlider);

    addLabelPairs(inGainSlider->labels, gainInParam, DB);
    addLabelPairs(lowMidXoverSlider->labels, lowMidCrossoverParam, HZ);
    addLabelPairs(midHighXoverSlider->labels, midHighCrossoverParam, HZ);
    addLabelPairs(outGainSlider->labels, gainOutParam, DB);

    addAndMakeVisible(*inGainSlider);
    addAndMakeVisible(*lowMidXoverSlider);
    addAndMakeVisible(*midHighXoverSlider);
    addAndMakeVisible(*outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void GlobalControls::resized() {
    auto bounds = getLocalBounds().reduced(DEFAULT_PADDING);
    using namespace juce;

    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(FLEX_SPACER);
    auto endCap = FlexItem().withWidth(FLEX_END_CAP);

    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inGainSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMidXoverSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outGainSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(endCap);

    flexBox.performLayout(bounds);
}
