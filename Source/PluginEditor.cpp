/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Placeholder::Placeholder() {
    // Use a random color for now
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
};

//==============================================================================
void GlobalControls::paint(juce::Graphics& g) {
    using namespace juce;
    auto bounds = getLocalBounds();
    // setting a border color of blue vioet, then black fill
    g.setColour(Colours::blueviolet);
    g.fillAll();

    auto localBounds = bounds;

    bounds.reduce(BORDER_THICKNESS, BORDER_THICKNESS);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);

    g.drawRect(localBounds);
}

//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (600, 500);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
}

//==============================================================================
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
