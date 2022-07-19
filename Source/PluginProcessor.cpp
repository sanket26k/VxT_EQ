/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VxT_EQAudioProcessor::VxT_EQAudioProcessor()
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
}

VxT_EQAudioProcessor::~VxT_EQAudioProcessor()
{
}

//==============================================================================
const juce::String VxT_EQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VxT_EQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VxT_EQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VxT_EQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VxT_EQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VxT_EQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VxT_EQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VxT_EQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VxT_EQAudioProcessor::getProgramName (int index)
{
    return {};
}

void VxT_EQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

template<int Idx, typename chainType, typename coeffType>
void VxT_EQAudioProcessor::update(chainType& cutChain, const coeffType& cutCoeff)
{
    *cutChain.get<Idx>().coefficients = *cutCoeff[Idx];
    cutChain.setBypassed<Idx>(false);
}

template<typename chainType, typename coeffType>
inline void VxT_EQAudioProcessor::updateCut(chainType& cutChain, const coeffType& cutCoeff, const Slope cutSlope)
{
    cutChain.setBypassed<0>(true);
    cutChain.setBypassed<1>(true);
    cutChain.setBypassed<2>(true);
    cutChain.setBypassed<3>(true);

    switch (cutSlope)
    {
        case Slope_48:
        {
            update<3>(cutChain, cutCoeff);
            [[fallthrough]];
        }
        case Slope_36:
        {
            update<2>(cutChain, cutCoeff);
            [[fallthrough]];
        }
        case Slope_24:
        {
            update<1>(cutChain, cutCoeff);
            [[fallthrough]];
        }
        case Slope_12:
        {
            update<0>(cutChain, cutCoeff);
            break;
        }
    }
}

//==============================================================================
void VxT_EQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    leftChain.prepare(spec);
    rightChain.prepare(spec);

    updateFilters();
}

void VxT_EQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VxT_EQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void VxT_EQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // zeroing o/p
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateFilters();
    
    juce::dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getSingleChannelBlock(Channels::left);
    auto rightBlock = block.getSingleChannelBlock(Channels::right);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);


    leftChain.process(leftContext);
    rightChain.process(rightContext);

}

//==============================================================================
bool VxT_EQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VxT_EQAudioProcessor::createEditor()
{
    //return new juce::GenericAudioProcessorEditor(*this);
    return new VxT_EQAudioProcessorEditor (*this);
}

//==============================================================================
void VxT_EQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void VxT_EQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        updateFilters();
    }
}

void VxT_EQAudioProcessor::updateFilters()
{
    auto s = getChainSettings(apvts);
    double sampleRate = getSampleRate();
    //apply peak
    auto peakCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, s.peakF, s.peakQ, juce::Decibels::decibelsToGain(s.peakGain));
    *leftChain.get<FilterPositions::Peak>().coefficients = *peakCoeffs;
    *rightChain.get<FilterPositions::Peak>().coefficients = *peakCoeffs;

    //apply lowcut
    auto lowCutCoeff = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        s.lowCutF, sampleRate, (s.lowCutSlope + 1) * 2);
    updateCut(
        leftChain.get<FilterPositions::LowCut>(),
        lowCutCoeff,
        static_cast<Slope>(s.lowCutSlope)
    );
    updateCut(
        rightChain.get<FilterPositions::LowCut>(),
        lowCutCoeff,
        static_cast<Slope>(s.lowCutSlope)
    );

    //apply highcut
    auto highCutCoeff = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        s.highCutF, sampleRate, (s.highCutSlope + 1) * 2);
    updateCut(
        leftChain.get<FilterPositions::HighCut>(),
        highCutCoeff,
        static_cast<Slope>(s.highCutSlope)
    );
    updateCut(
        rightChain.get<FilterPositions::HighCut>(),
        highCutCoeff,
        static_cast<Slope>(s.highCutSlope)
    );
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState &apvts)
{
    ChainSettings s;

    s.lowCutF       = apvts.getRawParameterValue("LowCut")->load();
    s.lowCutSlope   = static_cast<Slope> (apvts.getRawParameterValue("LowCutSlope")->load());
    s.highCutF      = apvts.getRawParameterValue("HighCut")->load();
    s.highCutSlope  = static_cast<Slope> (apvts.getRawParameterValue("HighCutSlope")->load());
    s.peakF         = apvts.getRawParameterValue("Peak")->load();
    s.peakGain      = apvts.getRawParameterValue("PeakGain")->load();
    s.peakQ         = apvts.getRawParameterValue("PeakQ")->load();

    return s;
}


juce::AudioProcessorValueTreeState::ParameterLayout VxT_EQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::StringArray slopeArray;
    for (int i = 0; i < 4; i++)
    {
        juce::String str;
        str << (12 + i * 12);
        str << " dB/Oct";
        slopeArray.add(str);
    }
    // lowcut
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut", "LowCut",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 1.0f), 20.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCutSlope", "LowCutSlope",
        slopeArray, 1));

    //highcut
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut", "HighCut",
        juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 1.0f), 20000.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCutSlope", "HighCutSlope",
        slopeArray, 1));

    //peak
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak", "Peak",
        juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 1.0f), 1000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PeakGain", "PeakGain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 1.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PeakQ", "PeakQ",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f), 1.0f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VxT_EQAudioProcessor();
}
