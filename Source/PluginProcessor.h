/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include "Constants.h"
#include "DSP/CompressorBand.h"


//==============================================================================
// Note: this struct was not covered in the course, just had to copy it
// Used by GUI thread to process blocks
template<typename T>
struct Fifo {
    void prepare(int numChannels, int numSamples) {
        static_assert(std::is_same_v<T, juce::AudioBuffer<float>>, "prepare(numChannels, numSamples) should only be used when the Fifo is holding juce::AudioBuffer<float>");
        for (T& buffer : buffers) {
            buffer.setSize(numChannels,
                numSamples,
                false,       //clear everything
                true,        //including the extra space
                true);       //avoid reallocating if you can
            buffer.clear();
        }
    }

    void prepare(size_t numElements) {
        static_assert(std::is_same_v<T, std::vector<float>>, "prepare(numElements) should only be used when the Fifo is holding std::vector<float>");
        for (T& buffer : buffers) {
            buffer.clear();
            buffer.resize(numElements, 0);
        }
    }

    bool push(const T& t) {
        juce::AbstractFifo::ScopedWrite write = fifo.write(1);
        if (write.blockSize1 > 0) {
            buffers[write.startIndex1] = t;
            return true;
        }

        return false;
    }

    bool pull(T& t) {
        juce::AbstractFifo::ScopedRead read = fifo.read(1);
        if (read.blockSize1 > 0) {
            t = buffers[read.startIndex1];
            return true;
        }
        return false;
    }

    int getNumAvailableForReading() const {
        return fifo.getNumReady();
    }

private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo{ Capacity };
};

//==============================================================================
enum Channel {
    Right, // effectively 0
    Left // effectively 1
};

//==============================================================================
// Note: this struct was not covered in the course, just had to copy it
// Collect the buffers into blocks of fixed sizes using this class
// According to the course:
    // Host Buffer has x samples -> SingleChannelSampleFifo -> Fixed size Blocks -> Fast Fourier Transform DataGenerator ->
    // FastFourierTransform DataBlocks -> PathProducer -> Juce::Path -> which is consumed by the GUI to draw the Spectrum Analysis Curve
template<typename BlockType>
struct SingleChannelSampleFifo {
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch) {
        prepared.set(false);
    }

    void update(const BlockType& buffer) {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse);
        // auto is ok here because we don't nessecarily know the type of buffer (BlockType is a template)
        auto* channelPtr = buffer.getReadPointer(channelToUse);

        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    void prepare(int bufferSize) {
        prepared.set(false);
        size.set(bufferSize);

        bufferToFill.setSize(1,             //channel
            bufferSize,    //num samples
            false,         //keepExistingContent
            true,          //clear extra space
            true);         //avoid reallocating
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }
    //==============================================================================
    int getNumcompleteBuffersAvailable() const { return audioBufferFifo.getNumAvailableForReading(); }
    bool isPrepared() const { return prepared.get(); }
    int getSize() const { return size.get(); }
    //==============================================================================
    bool getAudioBuffer(BlockType& buf) { return audioBufferFifo.pull(buf); }

private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;

    void pushNextSampleIntoFifo(float sample) {
        if (fifoIndex == bufferToFill.getNumSamples()) {
            bool ok = audioBufferFifo.push(bufferToFill);
            juce::ignoreUnused(ok);
            fifoIndex = 0;
        }

        bufferToFill.setSample(0, fifoIndex, sample);
        ++fifoIndex;
    }
};

//==============================================================================
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

    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo{ Channel::Left };
    SingleChannelSampleFifo<BlockType> rightChannelFifo{ Channel::Right };

private:
    std::array<CompressorBand, 3> compressors;
    CompressorBand& lowBandComp = compressors[0];
    CompressorBand& midBandComp = compressors[1];
    CompressorBand& highBandComp = compressors[2];

    using Filter = juce::dsp::LinkwitzRileyFilter<float>;
    // this is being formatted this way to show the three bands
    // all filters induce a small delay, so we need the signal to travel through the same number of filters across all three bands
    // LP1 goes through an all pass and makes the low band
    // HP1 goes through another HP at a higher frequency for the high band, and goes through a LP for the mid band
    //     fc0  fc1
    Filter LP1, AP2,
           HP1, LP2,
                HP2;

    // Null test all pass filter
    //Filter invAP1, invAP2;
    //juce::AudioBuffer<float> invAPBuffer;

    juce::AudioParameterFloat* lowMidCrossover{ nullptr };
    juce::AudioParameterFloat* midHighCrossover{ nullptr };
    std::array < juce::AudioBuffer<float>, 3> filterBuffers;

    juce::dsp::Gain<float> inputGain, outputGain;
    juce::AudioParameterFloat* inputGainParam{ nullptr };
    juce::AudioParameterFloat* outputGainParam{ nullptr };

    template<typename T, typename U>
    void applyGain(T& buffer, U& gain) {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto ctx = juce::dsp::ProcessContextReplacing<float>(block);
        gain.process(ctx);
    }

    void updateState();
    void splitBands(const juce::AudioBuffer<float>& inputBuffer);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessor)
};
