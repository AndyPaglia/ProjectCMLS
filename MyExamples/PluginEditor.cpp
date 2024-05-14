/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LowPassFilterAudioProcessorEditor::LowPassFilterAudioProcessorEditor (LowPassFilterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setSize (400, 300);
    
    //******************************************************************************
    // 5. Initialize the sliders in the Editor constructor
    qualitySlider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    qualitySlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 100, 20);
    qualitySlider.setRange (0.0, 1.0, 0.1);
    qualityLabel.setText("Q",juce::dontSendNotification);

    frequencySlider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    frequencySlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 100, 20);
    frequencySlider.setRange (50.0, 20000.0, 100.0);
    frequencyLabel.setText("Frequency",juce::dontSendNotification);

    addAndMakeVisible (qualitySlider);
    addAndMakeVisible (qualityLabel);
    addAndMakeVisible (frequencySlider);
    addAndMakeVisible (frequencyLabel);
    
    //******************************************************************************
    // 8. Now we need to instantiate our attachments and linking the value tree state and the sliders
    // Since they are unique pointers we need to use the make_unique template
    // EX: SliderAttachment (AudioProcessorValueTreeState &stateToUse, const String &parameterID, Slider &slider)
    frequencySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "FREQ", frequencySlider);
    qualitySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Q", qualitySlider);

    //******************************************************************************

    
}

LowPassFilterAudioProcessorEditor::~LowPassFilterAudioProcessorEditor()
{
}

//==============================================================================
void LowPassFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void LowPassFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //*****************************************************************************
    // 6. Place them in the correct positions
    frequencySlider.setBounds(10,80,100,100);
    frequencyLabel.setBounds(10,50,130,20);

    qualitySlider.setBounds(200,80,100,100);
    qualityLabel.setBounds(200,50,130,20);

    //*****************************************************************************
   
}
