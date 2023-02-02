/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

/*
    DSP Roadmap
    1) figure out how to split the audio into 3 bands
    2) create parameters to control where this split happens
    3) prove that splitting into 3 bands produces no audible artifacts
    4) create audio parameeters for the 3 compressor bands. These need to live on each band instance.
    5) add 2 remaining compressors
    6) add ability to mute/solo/bypass individual compressors
    7) add input and output gain to offset changes in output level
    8) clean up anything that needs cleaning up
*/

#include <JuceHeader.h>
#include "Constants.h"

namespace Params {
    enum Names {
        LowMidCrossoverFreq,
        MidHighCrossoverFreq,

        ThresholdLowBand,
        ThresholdMidBand,
        ThresholdHighBand,

        AttackLowBand,
        AttackMidBand,
        AttackHighBand,

        ReleaseLowBand,
        ReleaseMidBand,
        ReleaseHighBand,

        RatioLowBand,
        RatioMidBand,
        RatioHighBand,

        BypassedLowBand,
        BypassedMidBand,
        BypassedHighBand,
    };

    inline const std::map<Names, juce::String>& GetParams() {
        static std::map<Names, juce::String> params = {
            { LowMidCrossoverFreq, "Low-Mid Crossover Freq" },
            { MidHighCrossoverFreq, "Mid-High Crossover Freq"},
            { ThresholdLowBand, "Threshold Low Band"},
            { ThresholdMidBand, "Threshold Mid Band"},
            { ThresholdHighBand, "Threshold High Band"},
            { AttackLowBand, "Attack Low Band"},
            { AttackMidBand, "Attack Mid Band"},
            { AttackHighBand, "Attack High Band"},
            { ReleaseLowBand, "Release Low Band"},
            { ReleaseMidBand, "Release Mid Band"},
            { ReleaseHighBand, "Release High Band"},
            { RatioLowBand, "Ratio Low Band"},
            { RatioMidBand, "Ratio Mid Band"},
            { RatioHighBand, "Ratio High Band"},
            { BypassedLowBand, "Bypassed Low Band"},
            { BypassedMidBand, "Bypassed Mid Band"},
            { BypassedHighBand, "Bypassed High Band"},
        };
        return params;
    }
}

struct CompressorBand {
public:
    juce::AudioParameterFloat* attack{ nullptr };
    juce::AudioParameterFloat* release{ nullptr };
    juce::AudioParameterFloat* threshold{ nullptr };
    juce::AudioParameterChoice* ratio{ nullptr };
    juce::AudioParameterBool* bypassed{ nullptr };

    void prepare(const juce::dsp::ProcessSpec& spec) {
        compressor.prepare(spec);
    }

    void updateCompressorSettings() {
        compressor.setAttack(attack->get());
        compressor.setRelease(release->get());
        compressor.setThreshold(threshold->get());
        compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue());
    }

    void process(juce::AudioBuffer<float>& buffer) {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);

        context.isBypassed = bypassed->get();

        compressor.process(context);
    }

private:
    juce::dsp::Compressor<float> compressor;
};

//==============================================================================
/**
*/
class SimpleMBCompAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleMBCompAudioProcessor();
    ~SimpleMBCompAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    static APVTS::ParameterLayout createParameterLayout();

    APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:
    CompressorBand compressor;

    using Filter = juce::dsp::LinkwitzRileyFilter<float>;
    Filter LP, HP;

    Filter AP;
    juce::AudioBuffer<float> apBuffer;

    juce::AudioParameterFloat* lowMidCrossover{ nullptr };
    std::array < juce::AudioBuffer<float>, 2> filterBuffers;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessor)
};
