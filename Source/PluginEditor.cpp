/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0), monoBuffer.getReadPointer(0, size), monoBuffer.getNumSamples() - size);
            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size), tempIncomingBuffer.getReadPointer(0, 0), size);

            fftDataGenerator.produceFFtDataForRendering(monoBuffer, ABSOLUTE_MINIMUM_GAIN); // Our scale only goes to -48dB, so we'll use that as our "negative infinity" for now
        }
    }

    // If there are FFT data buffers to pull, if we can pull a buffer, generate a path
    const auto fftSize = fftDataGenerator.getFFtSize();
    // 48000 sample rate / 2048 order = 23Hz resolution <- this is the bin width
    const double binWidth = sampleRate / (double)fftSize;

    while (fftDataGenerator.getNumAvailableFFTDataBlocks() > 0) {
        std::vector<float> fftData;
        if (fftDataGenerator.getFFTData(fftData)) {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, ABSOLUTE_MINIMUM_GAIN); // Our sepctrum graph only goes to -48dB, so ue that as the "negative infinity" for now
        }
    }

    // Pull the most recent path that has been produced, since that will be the most recent data to use - this is in case we can't pull the paths as fast as we make them
    while (pathProducer.getNumPathsAvailable()) {
        pathProducer.getPath(fftPath);
    }
}

//==============================================================================
SpectrumAnalyzer::SpectrumAnalyzer(SimpleMBCompAudioProcessor& p) :
    audioProcessor(p),
    leftPathProducer(audioProcessor.leftChannelFifo),
    rightPathProducer(audioProcessor.rightChannelFifo) {
    const juce::Array <juce::AudioProcessorParameter*>& params = audioProcessor.getParameters();
    for (juce::AudioProcessorParameter* param : params) {
        param->addListener(this);
    }

    startTimerHz(60);
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
    const juce::Array <juce::AudioProcessorParameter*>& params = audioProcessor.getParameters();
    for (juce::AudioProcessorParameter* param : params) {
        param->removeListener(this);
    }
}

void SpectrumAnalyzer::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);
}

void SpectrumAnalyzer::timerCallback() {
    if (shouldShowFFTAnlaysis) {
        juce::Rectangle<float> fftBounds = getAnalysisArea().toFloat();
        double sampleRate = audioProcessor.getSampleRate();

        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if (parametersChanged.compareAndSetBool(false, true)) {
        // DBG("params changed");
    }
    // Now need to repaint all the time since we have new spectrum analysis paths being made all the time
    repaint();
}

void SpectrumAnalyzer::paint(juce::Graphics& g) {
    // doing this so we don't have to write `juce::` everywhere in this function
    using namespace juce;
    g.fillAll(Colours::black);

    drawBackgroundGrid(g);

    Rectangle<int> responseArea = getAnalysisArea();

    if (shouldShowFFTAnlaysis) {
        Path leftChannelFFTPath = leftPathProducer.getPath();
        Path rightChannelFFTPath = rightPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        // draw the left path using sky blue
        g.setColour(Colours::skyblue);
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
        // draw the right path using light yellow
        g.setColour(Colours::lightyellow);
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }

    Path border;
    border.setUsingNonZeroWinding(false);
    border.addRoundedRectangle(getRenderArea(), ANALYSIS_AREA_PADDING);
    border.addRectangle(getLocalBounds());

    g.setColour(Colours::black);
    g.fillPath(border);
    drawTextLabels(g);
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
}

void SpectrumAnalyzer::resized() {
    using namespace juce;
}

void SpectrumAnalyzer::drawBackgroundGrid(juce::Graphics& g) {
    using namespace juce;
    auto freqs = getFrequencies();

    Rectangle<int> renderArea = getAnalysisArea();
    int left = renderArea.getX();
    int right = renderArea.getRight();
    int top = renderArea.getY();
    int bottom = renderArea.getBottom();
    int width = renderArea.getWidth();

    // cache the x value position into an array
    auto xs = getXs(freqs, left, width);
    
    g.setColour(Colours::dimgrey);
    for (float x : xs) {
        g.drawVerticalLine(x, top, bottom);
    }

    std::vector<float> gain = getGains();

    for (float gDb : gain) {
        float y = jmap(gDb, GAIN_MIN, GAIN_MAX, float(getHeight()), 0.f);
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey); // If gain is 0dB draw a green line, otherwise use dark grey
        g.drawHorizontalLine(y, left, right);
    }
}

void SpectrumAnalyzer::drawTextLabels(juce::Graphics& g) {
    using namespace juce;
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    Rectangle<int> renderArea = getAnalysisArea();
    int left = renderArea.getX();
    int top = renderArea.getY();
    int bottom = renderArea.getBottom();
    int width = renderArea.getWidth();

    auto freqs = getFrequencies();
    auto xs = getXs(freqs, left, width);

    for (int i = 0; i < freqs.size(); ++i) {
        float f = freqs[i];
        float x = xs[i];

        bool addK = false;
        String str;
        if (f > 999.f) {
            addK = true;
            f /= 1000.f;
        }
        str << f;
        if (addK) {
            str << "K";
        }
        str << "Hz";
        int textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        g.drawFittedText(str, r, juce::Justification::centred, NUMBER_OF_LINES_TEXT);
    }

    std::vector<float> gain = getGains();
    for (float gDb : gain) {
        float y = jmap(gDb, GAIN_MIN, GAIN_MAX, float(bottom), float(top));
        String str;
        if (gDb > 0) {
            str << "+";
        }
        str << gDb;

        int textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey); // If gain is 0dB draw a green line, otherwise use light grey
        g.drawFittedText(str, r, juce::Justification::centred, NUMBER_OF_LINES_TEXT);

        // draw labels on left side of response curve for the spectrum analyzer
        // the range here needs to go from 0dB to -48dB, so we can simply subtract 24dB from our existing gain array to get these numbers
        str.clear();
        str << (gDb - 24.f);
        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r, juce::Justification::centred, NUMBER_OF_LINES_TEXT);
    }
}

std::vector<float> SpectrumAnalyzer::getFrequencies() {
    return std::vector<float> {
        20, /*30, 40,*/ 50, 100,
        200, /*300, 400,*/ 500, 1000,
        2000, /*3000, 4000,*/ 5000, 10000,
        20000
    };
}

std::vector<float> SpectrumAnalyzer::getGains() {
    return std::vector<float>
    {
        -24, -12, 0, 12, 24
    };
}

std::vector<float> SpectrumAnalyzer::getXs(const std::vector<float>& freqs, float left, float width) {
    std::vector<float> xs;
    for (auto f : freqs) {
        auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
        xs.push_back(left + width * normX);
    }

    return xs;
}

juce::Rectangle<int> SpectrumAnalyzer::getRenderArea() {
    juce::Rectangle<int> bounds = getLocalBounds();

    /*bounds.reduce(JUCE_LIVE_CONSTANT(10),
        JUCE_LIVE_CONSTANT(8));*/
        //bounds.reduce(10, 8);
    bounds.removeFromTop(RESPONSE_CURVE_TOP_REMOVAL);
    bounds.removeFromBottom(RESPONSE_CURVE_BOTTOM_REMOVAL);
    bounds.removeFromLeft(RESPONSE_CURVE_SIDE_REMOVAL);
    bounds.removeFromRight(RESPONSE_CURVE_SIDE_REMOVAL);

    return bounds;
}

juce::Rectangle<int> SpectrumAnalyzer::getAnalysisArea() {
    juce::Rectangle<int> bounds = getRenderArea();
    bounds.removeFromTop(ANALYSIS_AREA_PADDING);
    bounds.removeFromBottom(ANALYSIS_AREA_PADDING);
    return bounds;
}

//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (600, 500);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void SimpleMBCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleMBCompAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    controlBar.setBounds(bounds.removeFromTop(CONTROL_BAR_HEIGHT));
    bandControls.setBounds(bounds.removeFromBottom(BAND_CONTROLS_HEIGHT));
    analyzer.setBounds(bounds.removeFromTop(ANALYZER_HEIGHT));
    globalControls.setBounds(bounds);
}
