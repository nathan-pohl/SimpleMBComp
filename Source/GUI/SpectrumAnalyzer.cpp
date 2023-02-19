/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 14 Feb 2023 3:59:36pm
    Author:  Nate

  ==============================================================================
*/

#include "SpectrumAnalyzer.h"
#include "../Constants.h"
#include "Utilities.h"
#include "../DSP/Params.h"

//==============================================================================
SpectrumAnalyzer::SpectrumAnalyzer(SimpleMBCompAudioProcessor& p) :
    audioProcessor(p),
    leftPathProducer(audioProcessor.leftChannelFifo),
    rightPathProducer(audioProcessor.rightChannelFifo) {
    const juce::Array <juce::AudioProcessorParameter*>& params = audioProcessor.getParameters();
    for (juce::AudioProcessorParameter* param : params) {
        param->addListener(this);
    }
    using namespace Params;
    const auto& paramNames = GetParams();
    
    auto floatHelper = [&apvts = audioProcessor.apvts, &paramNames](auto& param, const auto& paramName) {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(paramNames.at(paramName)));
        jassert(param != nullptr);
    };

    floatHelper(lowMidXoverParam, Names::LowMidCrossoverFreq);
    floatHelper(midHighXoverParam, Names::MidHighCrossoverFreq);

    floatHelper(lowThresholdParam, Names::ThresholdLowBand);
    floatHelper(midThresholdParam, Names::ThresholdMidBand);
    floatHelper(highThresholdParam, Names::ThresholdHighBand);

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
        auto bounds = getLocalBounds();
        juce::Rectangle<float> fftBounds = getAnalysisArea(bounds).toFloat();
        fftBounds.setBottom(bounds.getBottom());
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
    using namespace juce;
    g.fillAll(Colours::black);

    auto bounds = drawModuleBackground(g, getLocalBounds());

    drawBackgroundGrid(g, bounds);

    if (shouldShowFFTAnlaysis) {
        drawFFTAnalysis(g, bounds);
    }

    drawCrossovers(g, bounds);
    drawTextLabels(g, bounds);
}

void SpectrumAnalyzer::resized() {
    using namespace juce;
    auto bounds = getLocalBounds();
    auto fftBounds = getAnalysisArea(bounds).toFloat();
    auto negInf = jmap(bounds.toFloat().getBottom(), fftBounds.getBottom(), fftBounds.getY(), NEGATIVE_INFINITY, MAX_DECIBELS);
    // DBG("Negative Infinity: " << negInf);
    leftPathProducer.updateNegativeInfinity(negInf);
    rightPathProducer.updateNegativeInfinity(negInf);
}

void SpectrumAnalyzer::drawBackgroundGrid(juce::Graphics& g, juce::Rectangle<int> bounds) {
    using namespace juce;
    auto freqs = getFrequencies();

    Rectangle<int> renderArea = getAnalysisArea(bounds);
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
        float y = jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBELS, float(bottom), float(top));
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey); // If gain is 0dB draw a green line, otherwise use dark grey
        g.drawHorizontalLine(y, left, right);
    }
}

void SpectrumAnalyzer::drawTextLabels(juce::Graphics& g, juce::Rectangle<int> bounds) {
    using namespace juce;
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    Rectangle<int> renderArea = getAnalysisArea(bounds);
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
        r.setY(bounds.getY());

        g.drawFittedText(str, r, juce::Justification::centred, NUMBER_OF_LINES_TEXT);
    }

    std::vector<float> gain = getGains();
    for (float gDb : gain) {
        float y = jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBELS, float(bottom), float(top));
        String str;
        if (gDb > 0) {
            str << "+";
        }
        str << gDb;

        int textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        // draw bounds on right side
        r.setSize(textWidth, fontHeight);
        r.setX(bounds.getRight() - textWidth);
        r.setCentre(r.getCentreX(), y);
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey); // If gain is 0dB draw a green line, otherwise use light grey
        g.drawFittedText(str, r, juce::Justification::centred, NUMBER_OF_LINES_TEXT);

        // draw labels on left side of analyzer
        r.setX(bounds.getX() + 1);
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
    std::vector<float> values;

    auto increment = MAX_DECIBELS; // 12 db steps
    for (auto db = NEGATIVE_INFINITY; db <= MAX_DECIBELS; db += increment) {
        values.push_back(db);
    }

    return values;
}

std::vector<float> SpectrumAnalyzer::getXs(const std::vector<float>& freqs, float left, float width) {
    std::vector<float> xs;
    for (auto f : freqs) {
        auto normX = juce::mapFromLog10(f, MIN_FREQ, MAX_FREQ);
        xs.push_back(left + width * normX);
    }

    return xs;
}

juce::Rectangle<int> SpectrumAnalyzer::getRenderArea(juce::Rectangle<int> bounds) {
    /*bounds.reduce(JUCE_LIVE_CONSTANT(10),
        JUCE_LIVE_CONSTANT(8));*/
        //bounds.reduce(10, 8);
    bounds.removeFromTop(RESPONSE_CURVE_TOP_REMOVAL);
    bounds.removeFromBottom(RESPONSE_CURVE_BOTTOM_REMOVAL);
    bounds.removeFromLeft(RESPONSE_CURVE_SIDE_REMOVAL);
    bounds.removeFromRight(RESPONSE_CURVE_SIDE_REMOVAL);

    return bounds;
}

juce::Rectangle<int> SpectrumAnalyzer::getAnalysisArea(juce::Rectangle<int> bounds) {
    bounds = getRenderArea(bounds);
    bounds.removeFromTop(ANALYSIS_AREA_PADDING);
    bounds.removeFromBottom(ANALYSIS_AREA_PADDING);
    return bounds;
}

void SpectrumAnalyzer::drawFFTAnalysis(juce::Graphics& g, juce::Rectangle<int> bounds) {
    using namespace juce;
    Path leftChannelFFTPath = leftPathProducer.getPath();
    Path rightChannelFFTPath = rightPathProducer.getPath();
    Rectangle<int> responseArea = getAnalysisArea(bounds);
    Graphics::ScopedSaveState sss(g);
    g.reduceClipRegion(responseArea);

    leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), 0));
    rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), 0));

    // draw the left path using sky blue
    g.setColour(Colours::skyblue);
    g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
    // draw the right path using light yellow
    g.setColour(Colours::lightyellow);
    g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
}

void SpectrumAnalyzer::drawCrossovers(juce::Graphics& g, juce::Rectangle<int> bounds) {
    using namespace juce;
    bounds = getAnalysisArea(bounds);

    const auto top = bounds.getY();
    const auto bottom = bounds.getBottom();
    const auto left = bounds.getX();
    const auto right = bounds.getRight();

    auto mapX = [left = bounds.getX(), width = bounds.getWidth()](float frequency) {
        auto normX = mapFromLog10(frequency, MIN_FREQ, MAX_FREQ);
        return left + width * normX;
    };

    auto mapY = [bottom, top](float db) {
        return jmap(db, NEGATIVE_INFINITY, MAX_DECIBELS, float(bottom), (float)top);
    };

    auto lowMidX = mapX(lowMidXoverParam->get());
    auto midHighX = mapX(midHighXoverParam->get());
    g.setColour(Colours::orange);
    g.drawVerticalLine(lowMidX, top, bottom);
    g.drawVerticalLine(midHighX, top, bottom);

    auto zeroDb = mapY(0.f);
    g.setColour(Colours::hotpink.withAlpha(0.3f));
    g.fillRect(Rectangle<float>::leftTopRightBottom(left, zeroDb, lowMidX, mapY(lowBandGR)));
    g.fillRect(Rectangle<float>::leftTopRightBottom(lowMidX, zeroDb, midHighX, mapY(midBandGR)));
    g.fillRect(Rectangle<float>::leftTopRightBottom(midHighX, zeroDb, right, mapY(highBandGR)));

    g.setColour(Colours::yellow);
    g.drawHorizontalLine(mapY(lowThresholdParam->get()), left, lowMidX);
    g.drawHorizontalLine(mapY(midThresholdParam->get()), lowMidX, midHighX);
    g.drawHorizontalLine(mapY(highThresholdParam->get()), midHighX, right);
}

void SpectrumAnalyzer::update(const std::vector<float>& values) {
    jassert(values.size() == 6);
    enum {
        LowBandIn,
        LowBandOut,
        MidBandIn,
        MidBandOut,
        HighBandIn,
        HighBandOut
    };

    lowBandGR = values[LowBandOut] - values[LowBandIn];
    midBandGR = values[MidBandOut] - values[MidBandIn];
    highBandGR = values[HighBandOut] - values[HighBandIn];

    repaint();
}