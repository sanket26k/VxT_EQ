/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VxT_EQAudioProcessorEditor::VxT_EQAudioProcessorEditor (VxT_EQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto*comp : getComps())
        addAndMakeVisible(comp);
    

    setSize (800, 600);
}

VxT_EQAudioProcessorEditor::~VxT_EQAudioProcessorEditor()
{
}

//==============================================================================
void VxT_EQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void VxT_EQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    // resp.setBounds(responseArea);
    
    auto slopeArea = bounds.removeFromBottom(bounds.getHeight() * 0.1);
    lowSlope.setBounds(slopeArea.removeFromLeft(slopeArea.getWidth() * 0.2));
    highSlope.setBounds(slopeArea.removeFromRight(slopeArea.getWidth() * 0.25));


    auto lowCutArea  = bounds.removeFromLeft(bounds.getWidth() * 0.2);
    lowC.setBounds(lowCutArea);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.25);
    highC.setBounds(highCutArea);

    peakF.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.33));
    peakG.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.5));
    peakQ.setBounds(bounds.removeFromLeft(bounds.getWidth()));



}

std::vector<juce::Component*> VxT_EQAudioProcessorEditor::getComps()
{

    return {
        &peakF,
        &peakG,
        &peakQ,
        &lowC,
        &highC,
        &lowSlope,
        &highSlope
    };
}
