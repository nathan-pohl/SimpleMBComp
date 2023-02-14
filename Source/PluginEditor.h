/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/LookAndFeel.h"
#include "GUI/UtilityComponents.h"
#include "GUI/GlobalControls.h"
#include "GUI/CompressorBandControls.h"


enum FFTOrder {
    // Splits spectrum of 20Hz - 20000Hz into N equally sized frequency bins
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

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

//==============================================================================
// Generates the Path data for the Spectrum Analysis by being fed the fft data
template<typename PathType>
struct AnalyzerPathGenerator {
    // Converts 'renderData[]' into a juce::Path
    void generatePath(const std::vector<float>& renderData, juce::Rectangle<float> fftBounds, int fftSize, float binWidth, float negativeInfinity) {
        float top = fftBounds.getY();
        float bottom = fftBounds.getHeight();
        float width = fftBounds.getWidth();
        int numBins = (int)fftSize / 2;
        PathType p;
        p.preallocateSpace(3 * (int)fftBounds.getWidth());

        auto map = [bottom, top, negativeInfinity](float v) {
            return juce::jmap(v, negativeInfinity, 0.f, float(bottom), top);
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

//==============================================================================
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

    Placeholder controlBar;
    GlobalControls globalControls { audioProcessor.apvts };
    CompressorBandControls bandControls { audioProcessor.apvts };
    SpectrumAnalyzer analyzer{ audioProcessor };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessorEditor)
};
