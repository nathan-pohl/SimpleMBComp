/*
  ==============================================================================

    FFTDataGenerator.h
    Created: 14 Feb 2023 3:57:49pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
// Fast Fourier Transform for converting audio buffer data into FastFourierTransform DataBlocks
// According to the course:
    // Host Buffer has x samples -> SingleChannelSampleFifo -> Fixed size Blocks -> Fast Fourier Transform DataGenerator ->
    // FastFourierTransform DataBlocks -> PathProducer -> Juce::Path -> which is consumed by the GUI to draw the Spectrum Analysis Curve
template<typename BlockType>
struct FFTDataGenerator {
    /**
    Produces the FFT data from an audio buffer.
    */
    void produceFFtDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity) {
        const int fftSize = getFFtSize();
        fftData.assign(fftData.size(), 0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex + fftSize, fftData.begin());

        // first apply a windowing function to our data
        window->multiplyWithWindowingTable(fftData.data(), fftSize);        // [1]
        // then render our FFT data
        forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());   // [2]

        int numBins = (int)fftSize / 2;
        // TODO combine the below into the same for loop, probably makes no difference
        //normalize the fft values
        for (int i = 0; i < numBins; ++i) {
            fftData[i] /= (float)numBins;
        }

        //convert them to decibels
        for (int i = 0; i < numBins; ++i) {
            fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
        }

        fftDataFifo.push(fftData);
    }

    void changeOrder(FFTOrder newOrder) {
        // when you change order, recreate the window, forwardFFT, fifo, fftData
        // also reset the fifoIndex
        // things that need recreating should be created on the heap via std::make_unique<>
        order = newOrder;
        int fftSize = getFFtSize();

        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);

        fftData.clear();
        fftData.resize(fftSize * 2, 0);
        fftDataFifo.prepare(fftData.size());
    }

    //==============================================================================
    int getFFtSize() const { return 1 << order; }
    int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }
    //==============================================================================
    bool getFFTData(BlockType& fftData) { return fftDataFifo.pull(fftData); }

private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;

    Fifo<BlockType> fftDataFifo;
};
