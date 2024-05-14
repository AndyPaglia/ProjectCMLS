/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class LowPassFilterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    LowPassFilterAudioProcessorEditor (LowPassFilterAudioProcessor&);
    ~LowPassFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LowPassFilterAudioProcessor& audioProcessor;
    
    //******************************************************************************
    // 4. add the Sliders to the Editor
    juce::Slider qualitySlider;
    juce::Slider frequencySlider;

    juce::Label frequencyLabel;
    juce::Label qualityLabel;

    
    //******************************************************************************
    
     
    //*****************************************************************************
    // 7. Create a slider attachments to each slider using a unique pointer to objects of 
    // the class AudioProcessorValueTreeState::SliderAttachment
    // The sliderAttachments have to be created AFTER the sliders because when the destructor
    // will be called it will delete first the attachments and then the sliders!
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> qualitySliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencySliderAttachment;

    //*****************************************************************************


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowPassFilterAudioProcessorEditor)
};
