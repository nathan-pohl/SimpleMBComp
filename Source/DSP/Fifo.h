/*
  ==============================================================================

    Fifo.h
    Created: 14 Feb 2023 4:03:36pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
// Note: this struct was not covered in the course, just had to copy it
// Used by GUI thread to process blocks
template<typename T>
struct Fifo {
    void prepare(int numChannels, int numSamples) {
        static_assert(std::is_same_v<T, juce::AudioBuffer<float>>, "prepare(numChannels, numSamples) should only be used when the Fifo is holding juce::AudioBuffer<float>");
        for (T& buffer : buffers) {
            buffer.setSize(numChannels,
                numSamples,
                false,       //clear everything
                true,        //including the extra space
                true);       //avoid reallocating if you can
            buffer.clear();
        }
    }

    void prepare(size_t numElements) {
        static_assert(std::is_same_v<T, std::vector<float>>, "prepare(numElements) should only be used when the Fifo is holding std::vector<float>");
        for (T& buffer : buffers) {
            buffer.clear();
            buffer.resize(numElements, 0);
        }
    }

    bool push(const T& t) {
        juce::AbstractFifo::ScopedWrite write = fifo.write(1);
        if (write.blockSize1 > 0) {
            buffers[write.startIndex1] = t;
            return true;
        }

        return false;
    }

    bool pull(T& t) {
        juce::AbstractFifo::ScopedRead read = fifo.read(1);
        if (read.blockSize1 > 0) {
            t = buffers[read.startIndex1];
            return true;
        }
        return false;
    }

    int getNumAvailableForReading() const {
        return fifo.getNumReady();
    }

private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo{ Capacity };
};
