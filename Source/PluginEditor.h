/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define NUM_PEAK_FILTERS 16

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

struct RespCurveComponent : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    RespCurveComponent(VxT_EQAudioProcessor&);
    ~RespCurveComponent() override;

    VxT_EQAudioProcessor& audioProcessor;

    void paint(juce::Graphics&) override;
    
    // listener
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {};
    
    // timer
    void timerCallback() override;
    juce::Atomic<bool> paramChanged{ true };

    monoChain respChain;

    // calc Peak
    template<int Idx>
    double calcPeakMagnitude(peakFilter& chain, const double freq, const double sampleRate);
    template<>
    double calcPeakMagnitude<0>(peakFilter& chain, const double freq, const double sampleRate);
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
    //void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VxT_EQAudioProcessor& audioProcessor;
    RespCurveComponent respCurveComponent;

    CustomSlider peakF, peakG, peakQ, highC, lowC;
    CustomSlopeBox lowSlope, highSlope;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    using BoxAttachment = APVTS::ComboBoxAttachment;

    Attachment peakFA, peakGA, peakQA, highCA, lowCA;
    BoxAttachment lowSlopeA, highSlopeA;

    std::vector<juce::Component*> getComps();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VxT_EQAudioProcessorEditor)
};


