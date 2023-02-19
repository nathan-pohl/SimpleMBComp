/*
  ==============================================================================

    AnalyzerPathGenerator.h
    Created: 14 Feb 2023 4:00:06pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../DSP/Fifo.h"

//==============================================================================
// Generates the Path data for the Spectrum Analysis by being fed the fft data
template<typename PathType>
struct AnalyzerPathGenerator {
    // Converts 'renderData[]' into a juce::Path
    void generatePath(const std::vector<float>& renderData, juce::Rectangle<float> fftBounds, int fftSize, float binWidth, float negativeInfinity) {
        float top = fftBounds.getY();
        float bottom = fftBounds.getBottom();
        float width = fftBounds.getWidth();
        int numBins = (int)fftSize / 2;
        PathType p;
        p.preallocateSpace(3 * (int)fftBounds.getWidth());

        auto map = [bottom, top, negativeInfinity](float v) {
            return juce::jmap(v, negativeInfinity, MAX_DECIBELS, float(bottom), top);
        };

        auto y = map(renderData[0]);
        // assert that y is a number and is not infinite
        // jassert(!std::isnan(y) && !std::isinf(y));
        if (std::isnan(y) || std::isinf(y)) {
            y = bottom;
        }
        // start the path here
        p.startNewSubPath(0, y);
        const int pathResolution = 2; // you can draw line-to's every 'pathResolution' pixels.

        // create the rest of the path here
        for (int binNum = 1; binNum < numBins; binNum += pathResolution) {
            y = map(renderData[binNum]);
            // assert that y is a number and is not infinite
            jassert(!std::isnan(y) && !std::isinf(y));

            if (!std::isnan(y) && !std::isinf(y)) {
                float binFreq = binNum * binWidth;
                float normalizedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
                int binX = std::floor(normalizedBinX * width);
                p.lineTo(binX, y);
            }
        }
        pathFifo.push(p);
    }

    int getNumPathsAvailable() const {
        return pathFifo.getNumAvailableForReading();
    }

    bool getPath(PathType& path) {
        return pathFifo.pull(path);
    }
private:
    Fifo<PathType> pathFifo;
};
