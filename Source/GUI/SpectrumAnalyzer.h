/*
  ==============================================================================

    SpectrumAnalyzer.h
    Created: 14 Feb 2023 3:59:36pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "PathProducer.h"

//==============================================================================
struct SpectrumAnalyzer : juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer
{
public:
    SpectrumAnalyzer(SimpleMBCompAudioProcessor&);
    ~SpectrumAnalyzer();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { };
    void timerCallback() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void toggleAnalysisEnablement(bool enabled) {
        shouldShowFFTAnlaysis = enabled;
    }
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleMBCompAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged{ false };

    void drawBackgroundGrid(juce::Graphics& g);
    void drawTextLabels(juce::Graphics& g);

    std::vector<float> getFrequencies();
    std::vector<float> getGains();
    std::vector<float> getXs(const std::vector<float>& freqs, float left, float width);

    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();

    PathProducer leftPathProducer, rightPathProducer;
    bool shouldShowFFTAnlaysis = true;
};
