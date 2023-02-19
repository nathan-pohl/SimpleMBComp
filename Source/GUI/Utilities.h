/*
  ==============================================================================

    Utilities.h
    Created: 12 Feb 2023 4:06:56pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

enum FFTOrder {
    // Splits spectrum of 20Hz - 20000Hz into N equally sized frequency bins
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

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
    labels.add({ 0.f, getValString(param, true, suffix) }); // Minimum value at 0%
    labels.add({ 1.f, getValString(param, false, suffix) }); // Maximum value at 100%
}

juce::Rectangle<int> drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds);