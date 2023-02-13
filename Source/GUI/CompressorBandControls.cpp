/*
  ==============================================================================

    CompressorBandControls.cpp
    Created: 12 Feb 2023 4:11:32pm
    Author:  Nate

  ==============================================================================
*/

#include "CompressorBandControls.h"
#include "Utilities.h"
#include "../DSP/Params.h"

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
    bypassButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::yellow);
    bypassButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    soloButton.setName(SOLO_LABEL);
    soloButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
    soloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    muteButton.setName(MUTE_LABEL);
    muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);

    lowBand.setName(LOW_BUTTON_LABEL);
    lowBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    lowBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    midBand.setName(MID_BUTTON_LABEL);
    midBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    midBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    highBand.setName(HIGH_BUTTON_LABEL);
    highBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    highBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

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
    updateSliderEnablements();
    updateBandSelectButtonStates();

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
    updateActiveBandFillColors(*button);
}

void CompressorBandControls::updateActiveBandFillColors(juce::Button& clickedButton) {
    jassert(activeBand != nullptr);
    if (clickedButton.getToggleState() == false) {
        resetActiveBandColors();
    }
    else {
        refreshBandButtonColors(*activeBand, clickedButton);
    }
}

void CompressorBandControls::resetActiveBandColors() {
    activeBand->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    activeBand->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    activeBand->repaint();
}

void CompressorBandControls::refreshBandButtonColors(juce::Button& band, juce::Button& colorSource) {
    band.setColour(juce::TextButton::ColourIds::buttonOnColourId, colorSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    band.setColour(juce::TextButton::ColourIds::buttonColourId, colorSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    band.repaint();
}

void CompressorBandControls::updateBandSelectButtonStates() {
    using namespace Params;
    std::vector<std::array<Names, 3>> paramsToCheck{
        {Names::SoloLowBand, Names::MuteLowBand, Names::BypassedLowBand},
        {Names::SoloMidBand, Names::MuteMidBand, Names::BypassedMidBand},
        {Names::SoloHighBand, Names::MuteHighBand, Names::BypassedHighBand}
    };

    const auto& params = GetParams();
    auto paramHelper = [&params, this](const auto& name) {
        return dynamic_cast<juce::AudioParameterBool*>(&getParam(apvts, params, name));
    };

    for (size_t i = 0; i < paramsToCheck.size(); ++i) {
        auto& list = paramsToCheck[i];
        auto* bandButton = (i == 0) ? &lowBand :
                           (i == 1) ? &midBand :
                                      &highBand;

        if (auto* solo = paramHelper(list[0]); solo->get()) {
            refreshBandButtonColors(*bandButton, soloButton);
        }
        else if (auto* mute = paramHelper(list[1]); mute->get()) {
            refreshBandButtonColors(*bandButton, muteButton);
        }
        else if (auto* byp = paramHelper(list[2]); byp->get()) {
            refreshBandButtonColors(*bandButton, bypassButton);
        }
    }
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
        activeBand = &lowBand;
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
        activeBand = &midBand;
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
        activeBand = &highBand;
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