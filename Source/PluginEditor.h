/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomSlider : juce::Slider
{
    CustomSlider() : juce::Slider(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextBoxBelow)
    {
        
    }
};

struct CustomSlopeBox : juce::ComboBox
{
    CustomSlopeBox() : juce::ComboBox()
    {
        this->addItem("-12", 1);
        this->addItem("-24", 2);
        this->addItem("-36", 3);
        this->addItem("-48", 4);

    }
};

//==============================================================================
/**
*/
class VxT_EQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VxT_EQAudioProcessorEditor (VxT_EQAudioProcessor&);
    ~VxT_EQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VxT_EQAudioProcessor& audioProcessor;

    CustomSlider peakF, peakG, peakQ, highC, lowC;
    CustomSlopeBox lowSlope, highSlope;

    using APVTS = juce::AudioProcessorValueTreeState;


    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VxT_EQAudioProcessorEditor)
};
