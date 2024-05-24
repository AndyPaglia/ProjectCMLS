/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class CMLSJuceAudioProcessor  : public juce::AudioProcessor, private juce::OSCReceiver,
private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    CMLSJuceAudioProcessor();
    ~CMLSJuceAudioProcessor() override;

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
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState tree_state {*this, nullptr, "Parameters", createLayout()};

  private:
    using Filter = juce::dsp::StateVariableTPTFilter<float>;
    float Fs;

    Filter F1; 
    Filter F2;
    Filter F3;

    std::vector<juce::AudioBuffer<float>> L_bands,R_bands;

    float f1_band = 500, f1_min = 250;
    float f2_band = 1640, f2_min = 650;
    float f3_band = 630, f3_min = 2300;

    uint8_t midi[2];

    float freq1 = 500, freq2 = 1000, freq3 = 2000;

    void oscMessageReceived (const juce::OSCMessage& message) override;
    void calcFreqs();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CMLSJuceAudioProcessor)
};
