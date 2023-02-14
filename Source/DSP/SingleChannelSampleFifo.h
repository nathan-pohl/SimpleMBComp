/*
  ==============================================================================

    SingleChannelSampleFifo.h
    Created: 14 Feb 2023 4:03:53pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Fifo.h"

//==============================================================================
enum Channel {
    Right, // effectively 0
    Left // effectively 1
};

//==============================================================================
// Note: this struct was not covered in the course, just had to copy it
// Collect the buffers into blocks of fixed sizes using this class
// According to the course:
    // Host Buffer has x samples -> SingleChannelSampleFifo -> Fixed size Blocks -> Fast Fourier Transform DataGenerator ->
    // FastFourierTransform DataBlocks -> PathProducer -> Juce::Path -> which is consumed by the GUI to draw the Spectrum Analysis Curve
template<typename BlockType>
struct SingleChannelSampleFifo {
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch) {
        prepared.set(false);
    }

    void update(const BlockType& buffer) {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse);
        // auto is ok here because we don't nessecarily know the type of buffer (BlockType is a template)
        auto* channelPtr = buffer.getReadPointer(channelToUse);

        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    void prepare(int bufferSize) {
        prepared.set(false);
        size.set(bufferSize);

        bufferToFill.setSize(1,             //channel
            bufferSize,    //num samples
            false,         //keepExistingContent
            true,          //clear extra space
            true);         //avoid reallocating
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }
    //==============================================================================
    int getNumcompleteBuffersAvailable() const { return audioBufferFifo.getNumAvailableForReading(); }
    bool isPrepared() const { return prepared.get(); }
    int getSize() const { return size.get(); }
    //==============================================================================
    bool getAudioBuffer(BlockType& buf) { return audioBufferFifo.pull(buf); }

private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;

    void pushNextSampleIntoFifo(float sample) {
        if (fifoIndex == bufferToFill.getNumSamples()) {
            bool ok = audioBufferFifo.push(bufferToFill);
            juce::ignoreUnused(ok);
            fifoIndex = 0;
        }

        bufferToFill.setSample(0, fifoIndex, sample);
        ++fifoIndex;
    }
};
