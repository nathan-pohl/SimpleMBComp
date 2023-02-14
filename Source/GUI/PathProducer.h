/*
  ==============================================================================

    PathProducer.h
    Created: 14 Feb 2023 3:59:50pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "Utilities.h"
#include "../DSP/SingleChannelSampleFifo.h"
#include "FFTDataGenerator.h"
#include "AnalyzerPathGenerator.h"

//==============================================================================
struct PathProducer {
    PathProducer(SingleChannelSampleFifo<SimpleMBCompAudioProcessor::BlockType>& scsf) : channelFifo(&scsf) {
        // use order of 8192 for best resolution of lower end of spectrum
        // e.g. 48000 sample rate / 8192 order = 6Hz resolution
        // using higher order rates gives better resolution at lower frequencies, at the expense of more CPU
        fftDataGenerator.changeOrder(FFTOrder::order8192);
        monoBuffer.setSize(1, fftDataGenerator.getFFtSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return fftPath; }
private:
    SingleChannelSampleFifo<SimpleMBCompAudioProcessor::BlockType>* channelFifo;
    juce::AudioBuffer<float> monoBuffer;
    FFTDataGenerator<std::vector<float>> fftDataGenerator;
    AnalyzerPathGenerator<juce::Path> pathProducer;
    juce::Path fftPath;
};