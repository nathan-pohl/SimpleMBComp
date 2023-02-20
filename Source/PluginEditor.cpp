/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/Params.h"

//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&lookAndFeel);

    controlBar.analyzerButton.onClick = [this]() {
        bool shouldBeOn = controlBar.analyzerButton.getToggleState();
        analyzer.toggleAnalysisEnablement(shouldBeOn);
    };

    controlBar.globalBypassButton.onClick = [this]() {
        toggleGlobalBypassState();
    };

    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (600, 500);
    startTimerHz(60);
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

void SimpleMBCompAudioProcessorEditor::timerCallback() {
    std::vector<float> values{
        audioProcessor.lowBandComp.getRmsInputLevelDb(),
        audioProcessor.lowBandComp.getRmsOutputLevelDb(),
        audioProcessor.midBandComp.getRmsInputLevelDb(),
        audioProcessor.midBandComp.getRmsOutputLevelDb(),
        audioProcessor.highBandComp.getRmsInputLevelDb(),
        audioProcessor.highBandComp.getRmsOutputLevelDb(),
    };

    analyzer.update(values);
    updateGlobalBypassButton();
}

void SimpleMBCompAudioProcessorEditor::toggleGlobalBypassState() {
    bool shouldEnableEverything = !controlBar.globalBypassButton.getToggleState();
    auto params = getBypassParams();

    auto bypassParamHelper = [](auto* param, bool shouldBeBypassed) {
        param->beginChangeGesture();
        param->setValueNotifyingHost(shouldBeBypassed ? 1.f : 0.f);
        param->endChangeGesture();
    };

    for (auto* param : params) {
        bypassParamHelper(param, !shouldEnableEverything);
    }
    bandControls.toggleAllBands(!shouldEnableEverything);
}

std::array<juce::AudioParameterBool*, 3> SimpleMBCompAudioProcessorEditor::getBypassParams() {
    using namespace Params;
    using namespace juce;
    const auto& params = GetParams();
    auto& apvts = audioProcessor.apvts;

    auto boolHelper = [&apvts, &params](const auto& paramName) {
        auto param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);

        return param;
    };

    auto* lowBypassParam = boolHelper(Names::BypassedLowBand);
    auto* midBypassParam = boolHelper(Names::BypassedMidBand);
    auto* highBypassParam = boolHelper(Names::BypassedHighBand);

    return {
        lowBypassParam,
        midBypassParam,
        highBypassParam
    };
}

void SimpleMBCompAudioProcessorEditor::updateGlobalBypassButton() {
    auto params = getBypassParams();
    bool allBandsAreBypassed = std::all_of(params.begin(), params.end(), [](const auto& param) {return param->get(); });
    controlBar.globalBypassButton.setToggleState(allBandsAreBypassed, juce::NotificationType::dontSendNotification);
}
