/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleMBCompAudioProcessor::SimpleMBCompAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    using namespace Params;
    const auto& params = GetParams();

    auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName) {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    auto choiceHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName) {
        param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    auto boolHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName) {
        param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    floatHelper(lowBandComp.attack, Names::AttackLowBand);
    floatHelper(lowBandComp.release, Names::ReleaseLowBand);
    floatHelper(lowBandComp.threshold, Names::ThresholdLowBand);
    choiceHelper(lowBandComp.ratio, Names::RatioLowBand);
    boolHelper(lowBandComp.bypassed, Names::BypassedLowBand);
    boolHelper(lowBandComp.mute, Names::MuteLowBand);
    boolHelper(lowBandComp.solo, Names::SoloLowBand);

    floatHelper(midBandComp.attack, Names::AttackMidBand);
    floatHelper(midBandComp.release, Names::ReleaseMidBand);
    floatHelper(midBandComp.threshold, Names::ThresholdMidBand);
    choiceHelper(midBandComp.ratio, Names::RatioMidBand);
    boolHelper(midBandComp.bypassed, Names::BypassedMidBand);
    boolHelper(midBandComp.mute, Names::MuteMidBand);
    boolHelper(midBandComp.solo, Names::SoloMidBand);

    floatHelper(highBandComp.attack, Names::AttackHighBand);
    floatHelper(highBandComp.release, Names::ReleaseHighBand);
    floatHelper(highBandComp.threshold, Names::ThresholdHighBand);
    choiceHelper(highBandComp.ratio, Names::RatioHighBand);
    boolHelper(highBandComp.bypassed, Names::BypassedHighBand);
    boolHelper(highBandComp.mute, Names::MuteHighBand);
    boolHelper(highBandComp.solo, Names::SoloHighBand);

    floatHelper(lowMidCrossover, Names::LowMidCrossoverFreq);
    floatHelper(midHighCrossover, Names::MidHighCrossoverFreq);

    floatHelper(inputGainParam, Names::GainIn);
    floatHelper(outputGainParam, Names::GainOut);

    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);

    //invAP1.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    //invAP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
}

SimpleMBCompAudioProcessor::~SimpleMBCompAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleMBCompAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleMBCompAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMBCompAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMBCompAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleMBCompAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleMBCompAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleMBCompAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleMBCompAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleMBCompAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleMBCompAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleMBCompAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    // passing compressors by reference for preparation
    for (auto& comp : compressors) {
        comp.prepare(spec);
    }

    LP1.prepare(spec);
    LP2.prepare(spec);
    HP1.prepare(spec); 
    HP2.prepare(spec);
    AP2.prepare(spec);

    // null test all pass
    //invAP1.prepare(spec);
    //invAP2.prepare(spec);
    //invAPBuffer.setSize(spec.numChannels, samplesPerBlock);

    inputGain.prepare(spec);
    outputGain.prepare(spec);

    inputGain.setRampDurationSeconds(0.05); // 50 ms
    outputGain.setRampDurationSeconds(0.05);

    for (auto& buffer : filterBuffers) {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }
}

void SimpleMBCompAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleMBCompAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleMBCompAudioProcessor::updateState() {
    for (auto& compressor : compressors) {
        compressor.updateCompressorSettings();
    }

    auto lowMidCutoff = lowMidCrossover->get();
    LP1.setCutoffFrequency(lowMidCutoff);
    HP1.setCutoffFrequency(lowMidCutoff);
    //invAP1.setCutoffFrequency(lowMidCutoff);

    auto midHighCutoff = midHighCrossover->get();
    AP2.setCutoffFrequency(midHighCutoff);
    LP2.setCutoffFrequency(midHighCutoff);
    HP2.setCutoffFrequency(midHighCutoff);
    //invAP2.setCutoffFrequency(midHighCutoff);

    inputGain.setGainDecibels(inputGainParam->get());
    outputGain.setGainDecibels(outputGainParam->get());
}

void SimpleMBCompAudioProcessor::splitBands(const juce::AudioBuffer<float>& inputBuffer) {
    // this wasn't explained in the video at all, but this for loop is actually populating the filterBuffers with
    // copies of the buffer using the reference to each buffer in the array with 'auto& fb'
    // this allows all the code below to let the LinkwitzRileyFilter work on the buffer
    // while leaving the signal intact when the filter outputs are summed.
    for (auto& fb : filterBuffers) {
        fb = inputBuffer;
    }

    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);

    // Ctx = context
    auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    auto fb2Ctx = juce::dsp::ProcessContextReplacing<float>(fb2Block);

    // all three buffers must be processed the same number of times
    // so each context must be ran through 2 filters to prevent delay artifacts
    LP1.process(fb0Ctx);
    AP2.process(fb0Ctx);

    HP1.process(fb1Ctx);
    filterBuffers[2] = filterBuffers[1]; // copy the processed buffer into the third buffer, so each buffer is processed twice still
    LP2.process(fb1Ctx);
    HP2.process(fb2Ctx);
}

void SimpleMBCompAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateState();

    applyGain(buffer, inputGain);

    // null test
    //invAPBuffer = buffer;

    splitBands(buffer);

    for (size_t i = 0; i < filterBuffers.size(); ++i) {
        compressors[i].process(filterBuffers[i]);
    }

    // null test using all pass
    //auto invAPBlock = juce::dsp::AudioBlock<float>(invAPBuffer);
    //auto invAPCtx = juce::dsp::ProcessContextReplacing<float>(invAPBlock);
    //invAP1.process(invAPCtx);
    //invAP2.process(invAPCtx);

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    buffer.clear();

    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source) {
        for (auto i = 0; i < nc; ++i) {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        }
    };

    auto bandsAreSoloed = false;
    for (auto& comp : compressors) {
        if (comp.solo->get()) {
            bandsAreSoloed = true;
            break;
        }
    }

    if (bandsAreSoloed) {
        for (size_t i = 0; i < compressors.size(); ++i) {
            auto& comp = compressors[i];
            if (comp.solo->get()) {
                addFilterBand(buffer, filterBuffers[i]);
            }
        }
    }
    else {
        for (size_t i = 0; i < compressors.size(); ++i) {
            auto& comp = compressors[i];
            if (!comp.mute->get()) {
                addFilterBand(buffer, filterBuffers[i]);
            }
        }
    }

    // null test using all pass
    //if (compressor.bypassed->get()) {
    //    for (auto ch = 0; ch < numChannels; ++ch) {
    //        juce::FloatVectorOperations::multiply(invAPBuffer.getWritePointer(ch), -1.f, numSamples);
    //    }
    //    addFilterBand(buffer, invAPBuffer);
    //}

    applyGain(buffer, outputGain);
}

//==============================================================================
bool SimpleMBCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleMBCompAudioProcessor::createEditor()
{
    // return new SimpleMBCompAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleMBCompAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void SimpleMBCompAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
}


juce::AudioProcessorValueTreeState::ParameterLayout SimpleMBCompAudioProcessor::createParameterLayout() {
    APVTS::ParameterLayout layout;
    using namespace juce;
    using namespace Params;
    const auto& params = GetParams();

    auto gainRange = NormalisableRange<float>(GAIN_MIN, GAIN_MAX, GAIN_INTERVAL, DEFAULT_SKEW_FACTOR);
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::GainIn), params.at(Names::GainIn), gainRange, GAIN_DEFAULT));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::GainOut), params.at(Names::GainOut), gainRange, GAIN_DEFAULT));

    auto thresholdRange = NormalisableRange<float>(THRESHOLD_MIN_VAL, THRESHOLD_MAX_VAL, DEFAULT_INTERVAL, DEFAULT_SKEW_FACTOR);
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::ThresholdLowBand), params.at(Names::ThresholdLowBand), thresholdRange, THRESHOLD_DEFAULT));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::ThresholdMidBand), params.at(Names::ThresholdMidBand), thresholdRange, THRESHOLD_DEFAULT));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::ThresholdHighBand), params.at(Names::ThresholdHighBand), thresholdRange, THRESHOLD_DEFAULT));

    auto attackReleaseRange = NormalisableRange<float>(ATTACK_RELEASE_MIN_VAL, ATTACK_RELEASE_MAX_VAL, DEFAULT_INTERVAL, DEFAULT_SKEW_FACTOR);
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::AttackLowBand), params.at(Names::AttackLowBand), attackReleaseRange, ATTACK_DEFAULT));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::AttackMidBand), params.at(Names::AttackMidBand), attackReleaseRange, ATTACK_DEFAULT));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::AttackHighBand), params.at(Names::AttackHighBand), attackReleaseRange, ATTACK_DEFAULT));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::ReleaseLowBand), params.at(Names::ReleaseLowBand), attackReleaseRange, RELEASE_DEFAULT));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::ReleaseMidBand), params.at(Names::ReleaseMidBand), attackReleaseRange, RELEASE_DEFAULT));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::ReleaseHighBand), params.at(Names::ReleaseHighBand), attackReleaseRange, RELEASE_DEFAULT));

    juce::StringArray sa;
    for (auto choice : RATIO_CHOICES) {
        sa.add(juce::String(choice, 1));
    }
    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::RatioLowBand), params.at(Names::RatioLowBand), sa, RATIO_DEFAULT));
    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::RatioMidBand), params.at(Names::RatioMidBand), sa, RATIO_DEFAULT));
    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::RatioHighBand), params.at(Names::RatioHighBand), sa, RATIO_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::BypassedLowBand), params.at(Names::BypassedLowBand), APVTS_BOOL_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::BypassedMidBand), params.at(Names::BypassedMidBand), APVTS_BOOL_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::BypassedHighBand), params.at(Names::BypassedHighBand), APVTS_BOOL_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::MuteLowBand), params.at(Names::MuteLowBand), APVTS_BOOL_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::MuteMidBand), params.at(Names::MuteMidBand), APVTS_BOOL_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::MuteHighBand), params.at(Names::MuteHighBand), APVTS_BOOL_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::SoloLowBand), params.at(Names::SoloLowBand), APVTS_BOOL_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::SoloMidBand), params.at(Names::SoloMidBand), APVTS_BOOL_DEFAULT));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::SoloHighBand), params.at(Names::SoloHighBand), APVTS_BOOL_DEFAULT));

    auto lowMidRange = NormalisableRange<float>(LOW_MID_MIN_FREQ, LOW_MID_MAX_FREQ, DEFAULT_INTERVAL, DEFAULT_SKEW_FACTOR);
    auto midHighRange = NormalisableRange<float>(MID_HIGH_MIN_FREQ, MID_HIGH_MAX_FREQ, DEFAULT_INTERVAL, DEFAULT_SKEW_FACTOR);

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::LowMidCrossoverFreq), params.at(Names::LowMidCrossoverFreq), lowMidRange, LOW_MID_CROSSOVER_DEFAULT_FREQUENCY));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::MidHighCrossoverFreq), params.at(Names::MidHighCrossoverFreq), midHighRange, MID_HIGH_CROSSOVER_DEFAULT_FREQUENCY));

    return layout;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleMBCompAudioProcessor();
}
