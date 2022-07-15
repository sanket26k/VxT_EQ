/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope {
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings {
    float lowCutF{ 0 };     Slope lowCutSlope{ Slope::Slope_24 };
    float highCutF{ 0 };    Slope highCutSlope{ Slope::Slope_24 };
    float peakF{ 0 };       float peakGain{ 0 };        float peakQ{ 1.0f };
};
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class VxT_EQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    VxT_EQAudioProcessor();
    ~VxT_EQAudioProcessor() override;

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

    // VxT EQ Public
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ 
        *this, nullptr, "Parameters", createParameterLayout() };


private:
    // VxT EQ Private
    using filter = juce::dsp::IIR::Filter<float>;
    using cutFilter = juce::dsp::ProcessorChain<filter, filter, filter, filter>;
    using monoChain = juce::dsp::ProcessorChain<cutFilter, filter, cutFilter>;
    monoChain leftChain, rightChain;
    enum Channels {left=0, right=1};
    enum FilterPositions {LowCut, Peak, HighCut};

    template<typename chainType, typename coeffType>
        void updateCut(chainType& cutChain, const coeffType& cutCoeff, const Slope cutSlope);
    template<int Idx, typename chainType, typename coeffType>
    void update(chainType& cutChain, const coeffType& cutCoeff);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VxT_EQAudioProcessor)
};
