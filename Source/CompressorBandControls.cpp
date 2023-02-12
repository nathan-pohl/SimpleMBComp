/*
  ==============================================================================

    CompressorBandControls.cpp
    Created: 12 Feb 2023 4:11:32pm
    Author:  Nate

  ==============================================================================
*/

#include "CompressorBandControls.h"
#include "Utilities.h"
#include "Params.h"

CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apvts) :
    apvts(apvts),
    attackSlider(nullptr, MS, ATTACK_LABEL),
    releaseSlider(nullptr, MS, RELEASE_LABEL),
    thresholdSlider(nullptr, DB, THRESHOLD_LABEL),
    ratioSlider(nullptr, "")
{
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);

    bypassButton.addListener(this);
    soloButton.addListener(this);
    muteButton.addListener(this);

    bypassButton.setName(BYPASSED_LABEL);
    soloButton.setName(SOLO_LABEL);
    muteButton.setName(MUTE_LABEL);

    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);

    lowBand.setName(LOW_BUTTON_LABEL);
    midBand.setName(MID_BUTTON_LABEL);
    highBand.setName(HIGH_BUTTON_LABEL);

    lowBand.setRadioGroupId(1);
    midBand.setRadioGroupId(1);
    highBand.setRadioGroupId(1);

    auto buttonSwitcher = [safePtr = this->safePtr]() {
        if (auto* c = safePtr.getComponent()) {
            c->updateAttachments();
        }
    };

    lowBand.onClick = buttonSwitcher;
    midBand.onClick = buttonSwitcher;
    highBand.onClick = buttonSwitcher;

    // make lowBand the default choice
    lowBand.setToggleState(true, juce::NotificationType::dontSendNotification);
    updateAttachments();

    addAndMakeVisible(lowBand);
    addAndMakeVisible(midBand);
    addAndMakeVisible(highBand);
}

CompressorBandControls::~CompressorBandControls() {
    bypassButton.removeListener(this);
    soloButton.removeListener(this);
    muteButton.removeListener(this);
}

void CompressorBandControls::buttonClicked(juce::Button* button) {
    updateSliderEnablements();
    updateSoloMuteBypassToggleStates(*button);
}

void CompressorBandControls::updateSliderEnablements() {
    auto disabled = muteButton.getToggleState() || bypassButton.getToggleState();
    attackSlider.setEnabled(!disabled);
    releaseSlider.setEnabled(!disabled);
    thresholdSlider.setEnabled(!disabled);
    ratioSlider.setEnabled(!disabled);
}

void CompressorBandControls::updateSoloMuteBypassToggleStates(juce::Button& clickedButton) {
    if (&clickedButton == &soloButton && soloButton.getToggleState()) {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if (&clickedButton == &muteButton && muteButton.getToggleState()) {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if (&clickedButton == &bypassButton && bypassButton.getToggleState()) {
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
}

void CompressorBandControls::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void CompressorBandControls::resized() {
    auto bounds = getLocalBounds().reduced(DEFAULT_PADDING);
    using namespace juce;

    auto createBandButtonControlBox = [](std::vector<Component*> comps) {
        FlexBox flexBox;
        flexBox.flexDirection = FlexBox::Direction::column;
        flexBox.flexWrap = FlexBox::Wrap::noWrap;

        auto spacer = FlexItem().withHeight(FLEX_HEIGHT_SPACER);

        for (auto* comp : comps) {
            flexBox.items.add(spacer);
            flexBox.items.add(FlexItem(*comp).withFlex(FLEX_DEFAULT));
        }
        flexBox.items.add(spacer);

        return flexBox;
    };

    auto bandButtonControlBox = createBandButtonControlBox({ &bypassButton, &soloButton, &muteButton });
    auto bandSelectControlBox = createBandButtonControlBox({ &lowBand, &midBand, &highBand });

    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(FLEX_SPACER);
    auto endCap = FlexItem().withWidth(FLEX_END_CAP);

    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(bandSelectControlBox).withWidth(FLEX_BAND_SELECT_WIDTH));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(attackSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(releaseSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(thresholdSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(ratioSlider).withFlex(FLEX_DEFAULT));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(bandButtonControlBox).withWidth(FLEX_BAND_CONTROL_WIDTH));

    flexBox.performLayout(bounds);
}

void CompressorBandControls::updateAttachments() {
    enum BandType {
        Low,
        Mid,
        High
    };

    BandType bandType = [this]() {
        if (lowBand.getToggleState()) {
            return BandType::Low;
        }
        if (midBand.getToggleState()) {
            return BandType::Mid;
        }
        return BandType::High;
    }();

    using namespace Params;
    std::vector<Names> names;

    switch (bandType) {
    case Low:
        names = std::vector<Names>{
            Names::AttackLowBand,
            Names::ReleaseLowBand,
            Names::ThresholdLowBand,
            Names::RatioLowBand,
            Names::MuteLowBand,
            Names::SoloLowBand,
            Names::BypassedLowBand
        };
        break;
    case Mid:
        names = std::vector<Names>{
            Names::AttackMidBand,
            Names::ReleaseMidBand,
            Names::ThresholdMidBand,
            Names::RatioMidBand,
            Names::MuteMidBand,
            Names::SoloMidBand,
            Names::BypassedMidBand
        };
        break;
    case High:
        names = std::vector<Names>{
            Names::AttackHighBand,
            Names::ReleaseHighBand,
            Names::ThresholdHighBand,
            Names::RatioHighBand,
            Names::MuteHighBand,
            Names::SoloHighBand,
            Names::BypassedHighBand
        };
        break;
    }

    enum Pos {
        Attack,
        Release,
        Threshold,
        Ratio,
        Mute,
        Solo,
        Bypass
    };

    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts = this->apvts, &names](const auto& pos) -> auto& {
        return getParam(apvts, params, names.at(pos));
    };

    // must reset attachments before we can make new ones
    attackSliderAttachment.reset();
    releaseSliderAttachment.reset();
    thresholdSliderAttachment.reset();
    ratioSliderAttachment.reset();
    bypassButtonAttachment.reset();
    soloButtonAttachment.reset();
    muteButtonAttachment.reset();

    auto& attackParam = getParamHelper(Pos::Attack);
    addLabelPairs(attackSlider.labels, attackParam, MS);
    attackSlider.changeParam(&attackParam);

    auto& releaseParam = getParamHelper(Pos::Release);
    addLabelPairs(releaseSlider.labels, releaseParam, MS);
    releaseSlider.changeParam(&releaseParam);

    auto& threshParam = getParamHelper(Pos::Threshold);
    addLabelPairs(thresholdSlider.labels, threshParam, DB);
    thresholdSlider.changeParam(&threshParam);

    ratioSlider.labels.clear();
    // We can't cleanly show the ends of these params, need to use strings of the ratios which will make sense to the user
    ratioSlider.labels.add({ 0.f, "1:1" });
    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&getParamHelper(Pos::Ratio));
    // this abomination gets the last choice which is 100, and adds a string to the end to make it look like a ratio
    ratioSlider.labels.add({ 1.0f, juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1" });
    ratioSlider.changeParam(ratioParam);

    auto makeAttachmentHelper = [&params, &apvts = this->apvts](auto& attachment, const auto& name, auto& slider) {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(attackSliderAttachment, names[Pos::Attack], attackSlider);
    makeAttachmentHelper(releaseSliderAttachment, names[Pos::Release], releaseSlider);
    makeAttachmentHelper(thresholdSliderAttachment, names[Pos::Threshold], thresholdSlider);
    makeAttachmentHelper(ratioSliderAttachment, names[Pos::Ratio], ratioSlider);
    makeAttachmentHelper(bypassButtonAttachment, names[Pos::Bypass], bypassButton);
    makeAttachmentHelper(soloButtonAttachment, names[Pos::Solo], soloButton);
    makeAttachmentHelper(muteButtonAttachment, names[Pos::Mute], muteButton);
}