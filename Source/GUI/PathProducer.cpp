/*
  ==============================================================================

    PathProducer.cpp
    Created: 14 Feb 2023 3:59:50pm
    Author:  Nate

  ==============================================================================
*/

#include "PathProducer.h"

//==============================================================================
// This is where we need to coordinate the SingleChannelSampleFifo, FastFourierTransform Data generator, Path Producer, and GUI for Spectrum Analysis
void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate) {
    juce::AudioBuffer<float> tempIncomingBuffer;

    // When consuming the buffer, we take a number of sampled points of the sample size, run the FFT algorithm on that block, 
    // then shift the buffer forward by that sample size to take on the next block of sample points
    while (channelFifo->getNumcompleteBuffersAvailable() > 0) {
        if (channelFifo->getAudioBuffer(tempIncomingBuffer)) {
            // first shift everything in the monoBuffer forward by however many samples are in the temp buffer
            int size = tempIncomingBuffer.getNumSamples();
            jassert(size <= monoBuffer.getNumSamples());
            size = juce::jmin(size, monoBuffer.getNumSamples());

            auto writePointer = monoBuffer.getWritePointer(0, 0);
            auto readPointer = monoBuffer.getReadPointer(0, size);

            std::copy(readPointer, readPointer + (monoBuffer.getNumSamples() - size), writePointer);

            //juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0), monoBuffer.getReadPointer(0, size), monoBuffer.getNumSamples() - size);
            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size), tempIncomingBuffer.getReadPointer(0, 0), size);

            fftDataGenerator.produceFFtDataForRendering(monoBuffer, negativeInfinity); // Our scale only goes to -48dB, so we'll use that as our "negative infinity" for now
        }
    }

    // If there are FFT data buffers to pull, if we can pull a buffer, generate a path
    const auto fftSize = fftDataGenerator.getFFtSize();
    // 48000 sample rate / 2048 order = 23Hz resolution <- this is the bin width
    const double binWidth = sampleRate / (double)fftSize;

    while (fftDataGenerator.getNumAvailableFFTDataBlocks() > 0) {
        std::vector<float> fftData;
        if (fftDataGenerator.getFFTData(fftData)) {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, negativeInfinity); // Our sepctrum graph only goes to -48dB, so ue that as the "negative infinity" for now
        }
    }

    // Pull the most recent path that has been produced, since that will be the most recent data to use - this is in case we can't pull the paths as fast as we make them
    while (pathProducer.getNumPathsAvailable()) {
        pathProducer.getPath(fftPath);
    }
}
