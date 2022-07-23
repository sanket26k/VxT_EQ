/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

RespCurveComponent::RespCurveComponent(VxT_EQAudioProcessor& p) : audioProcessor(p)
{
    for (auto param : audioProcessor.getParameters())
        param->addListener(this);

    startTimerHz(60);
}

RespCurveComponent::~RespCurveComponent()
{
    for (auto param : audioProcessor.getParameters())
        param->removeListener(this);
}

void RespCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    paramChanged.set(true);
}

void RespCurveComponent::timerCallback()
{
    if (paramChanged.compareAndSetBool(false, true))
    {
        updateFilters(getChainSettings(audioProcessor.apvts), audioProcessor.getSampleRate(), respChain);
        repaint();
    }
}

void RespCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Colours::darkslategrey);
    auto respArea = getLocalBounds();

    //auto respArea = bounds.removeFromTop(bounds.getHeight() * 0.66);

    auto w = respArea.getWidth();

    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags;
    mags.resize(w);

    for (int i = 0; i < w; i++)
    {
        double mag = 1.0f;
        auto freq = mapToLog10(double(i) / double(w), (double)20, (double)20000);

        if (!respChain.isBypassed<FilterPositions::Peak>())
            mag *= respChain.get<FilterPositions::Peak>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!respChain.get<FilterPositions::LowCut>().isBypassed<0>())
            mag *= respChain.get<FilterPositions::LowCut>().get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!respChain.get<FilterPositions::LowCut>().isBypassed<1>())
            mag *= respChain.get<FilterPositions::LowCut>().get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!respChain.get<FilterPositions::LowCut>().isBypassed<2>())
            mag *= respChain.get<FilterPositions::LowCut>().get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!respChain.get<FilterPositions::LowCut>().isBypassed<3>())
            mag *= respChain.get<FilterPositions::LowCut>().get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!respChain.get<FilterPositions::HighCut>().isBypassed<0>())
            mag *= respChain.get<FilterPositions::HighCut>().get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!respChain.get<FilterPositions::HighCut>().isBypassed<1>())
            mag *= respChain.get<FilterPositions::HighCut>().get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!respChain.get<FilterPositions::HighCut>().isBypassed<2>())
            mag *= respChain.get<FilterPositions::HighCut>().get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!respChain.get<FilterPositions::HighCut>().isBypassed<3>())
            mag *= respChain.get<FilterPositions::HighCut>().get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path respCurve;
    const double opMin = respArea.getBottom();
    const double opMax = respArea.getY();
    auto map = [opMin, opMax](double in) {
        return jmap(in, -24.0, +12.0, opMin, opMax);
    };
    
    respCurve.startNewSubPath(respArea.getX(), map(mags.front()));
    for (auto i = 1; i < mags.size(); i++)
    {
        respCurve.lineTo(respArea.getX() + i, map(mags[i]));
    }
    g.setColour(Colours::blueviolet);
    g.drawRoundedRectangle(respArea.toFloat(), 20.f, 2.f);
    g.setColour(Colours::antiquewhite);
    g.strokePath(respCurve, PathStrokeType(4));


}


//==============================================================================
VxT_EQAudioProcessorEditor::VxT_EQAudioProcessorEditor(VxT_EQAudioProcessor& p)
    : AudioProcessorEditor(&p), 
    audioProcessor(p),
    peakFA(audioProcessor.apvts, "Peak", peakF),
    peakGA(audioProcessor.apvts, "PeakGain", peakG),
    peakQA(audioProcessor.apvts, "PeakQ", peakQ),
    highCA(audioProcessor.apvts, "HighCut", highC),
    lowCA(audioProcessor.apvts, "LowCut", lowC),
    lowSlopeA(audioProcessor.apvts, "LowCutSlope", lowSlope),
    highSlopeA(audioProcessor.apvts, "HighCutSlope", highSlope),
    respCurveComponent(audioProcessor)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps())
        addAndMakeVisible(comp);

    setSize(800, 600);
}

VxT_EQAudioProcessorEditor::~VxT_EQAudioProcessorEditor()
{

}

void VxT_EQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    respCurveComponent.setBounds(responseArea);
    
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
        &highSlope,
        &respCurveComponent
    };
}


