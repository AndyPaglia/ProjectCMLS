/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include <memory>
#include "PluginEditor.h"

//==============================================================================
CMLSJuceAudioProcessor::CMLSJuceAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       )
{
}

CMLSJuceAudioProcessor::~CMLSJuceAudioProcessor()
{
}

//==============================================================================
const juce::String CMLSJuceAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CMLSJuceAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CMLSJuceAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CMLSJuceAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CMLSJuceAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CMLSJuceAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CMLSJuceAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CMLSJuceAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CMLSJuceAudioProcessor::getProgramName (int index)
{
    return {};
}

void CMLSJuceAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CMLSJuceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    Fs = sampleRate;
    juce::dsp::ProcessSpec s;
    s.maximumBlockSize = samplesPerBlock;
    s.numChannels= 1;
    s.sampleRate = sampleRate;

    Formant_1.prepare(s);
    Formant_1.reset();
    
    Formant_2.prepare(s);
    Formant_2.reset();
    
    Formant_2.prepare(s);
    Formant_2.reset();
}

void CMLSJuceAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CMLSJuceAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CMLSJuceAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    float in_magnitude = buffer.getMagnitude(0,buffer.getNumSamples());

    juce::AudioBuffer<float> copy1;
    copy1.makeCopyOf<float>(buffer,1); 
    juce::AudioBuffer<float> copy2;
    copy2.makeCopyOf<float>(buffer,1); 
    juce::AudioBuffer<float> copy3;
    copy3.makeCopyOf<float>(buffer,1); 

    juce::dsp::AudioBlock<float> block1(copy1);
    juce::dsp::AudioBlock<float> block2(copy2);
    juce::dsp::AudioBlock<float> block3(copy3);
    
    juce::dsp::AudioBlock<float> out(buffer);
    juce::dsp::AudioBlock<float> app(buffer);
    
    float F1_freq = *tree_state.getRawParameterValue("F1_Freq"); 
    float F1_gain = *tree_state.getRawParameterValue("F1_Gain");
    float F1_q = *tree_state.getRawParameterValue("F1_Q");

    float F2_freq = *tree_state.getRawParameterValue("F2_Freq"); 
    float F2_gain = *tree_state.getRawParameterValue("F2_Gain");
    float F2_q = *tree_state.getRawParameterValue("F2_Q");

    float F3_freq = *tree_state.getRawParameterValue("F3_Freq"); 
    float F3_gain = *tree_state.getRawParameterValue("F3_Gain");
    float F3_q = *tree_state.getRawParameterValue("F3_Q");

    Formant_1.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,F1_freq,F1_q);
    Formant_2.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,F2_freq,F2_q);
    Formant_3.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,F3_freq,F3_q);

    block1 *= 1/in_magnitude;
    block2 *= 1/in_magnitude;
    block3 *= 1/in_magnitude;

    auto context1 =juce::dsp::ProcessContextReplacing<float>(block1);
    auto context2 =juce::dsp::ProcessContextReplacing<float>(block2);
    auto context3 =juce::dsp::ProcessContextReplacing<float>(block3);

    Formant_1.process(context1);
    Formant_2.process(context2);
    Formant_3.process(context3);
    
    context1.getOutputBlock().multiplyBy(F1_gain);
    context2.getOutputBlock().multiplyBy(F2_gain);
    context3.getOutputBlock().multiplyBy(F3_gain);

    app.replaceWithSumOf(context1.getOutputBlock(), context2.getOutputBlock());
    out.replaceWithSumOf(app, context3.getOutputBlock());
    
    float out_magnitude = buffer.getMagnitude(0,buffer.getNumSamples());
}

//==============================================================================
bool CMLSJuceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CMLSJuceAudioProcessor::createEditor()
{
    return new CMLSJuceAudioProcessorEditor (*this);
}

//==============================================================================
void CMLSJuceAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CMLSJuceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CMLSJuceAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout CMLSJuceAudioProcessor::createLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Freq", "Formant 1 Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 20.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Freq", "Formant 2 Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 20.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Freq", "Formant 3 Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 20.f)); 
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Gain", "Formant 1 Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), 0.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Gain", "Formant 2 Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), 0.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Gain", "Formant 3 Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), 0.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Q", "Formant 1 Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.5f, 1.f), 1.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Q", "Formant 2 Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.5f, 1.f), 1.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Q", "Formant 3 Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.5f, 1.f), 1.f));
    return {layout.begin(), layout.end()};
}
