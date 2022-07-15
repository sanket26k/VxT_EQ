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

//==============================================================================
void VxT_EQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool VxT_EQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VxT_EQAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
    //return new VxT_EQAudioProcessorEditor (*this);
}

//==============================================================================
void VxT_EQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VxT_EQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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
    layout.add(std::make_unique<juce::AudioParameterFloat>("PeakFreq", "PeakFreq",
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
