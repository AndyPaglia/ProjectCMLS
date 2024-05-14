/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
//******************************************************************************
// 3. In the initialization list of the constructor add the initialization for the
// value tree state using the funcion createParameters() that we just defined
// EX: AudioProcessorValueTreeState (AudioProcessor &processorToConnectTo, UndoManager *undoManagerToUse, const Identifier &valueTreeType, ParameterLayout parameterLayout)
// 10. In the initialization list of the constructor we add the initialization for
// the IIR filter
// EX: Filter (CoefficientsPtr coefficientsToUse)
 
LowPassFilterAudioProcessor::LowPassFilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                       // 3
                       apvts(*this, nullptr, "Parameters", createParameters()),
                       // 10
                       lowPassFilter(juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 20000.0f, 0.1))

                       
                       
#endif
{
}

LowPassFilterAudioProcessor::~LowPassFilterAudioProcessor()
{
}

//==============================================================================
const juce::String LowPassFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LowPassFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LowPassFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LowPassFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LowPassFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LowPassFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LowPassFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LowPassFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LowPassFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void LowPassFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LowPassFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // *****************************************************************************
    // 11. In prepare to play we need to reset our filter and initialize the filter
    // EX: void prepare (const ProcessSpec &) noexcept
    // It takes as input a ProcessSpec structure, where used sampleRate and samplesPerBloc
    // need to be specified
    lastSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    lowPassFilter.prepare(spec);
    lowPassFilter.reset();

    // *****************************************************************************

}

void LowPassFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LowPassFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void LowPassFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    
    //******************************************************************************
    // 12. Actually apply the filtering using Audio Block and ProcessContextReplacing
    // we use AudioBlock as a container for the input buffer
    // EX: AudioBlock (AudioBuffer< OtherSampleType > &buffer)
    juce::dsp::AudioBlock <float> block (buffer);

    float freq = *apvts.getRawParameterValue("FREQ");
    float quality = *apvts.getRawParameterValue("Q");

    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(lastSampleRate, freq, quality);
    
    lowPassFilter.process(juce::dsp::ProcessContextReplacing<float> (block));

    //******************************************************************************

}

//==============================================================================
bool LowPassFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LowPassFilterAudioProcessor::createEditor()
{
    return new LowPassFilterAudioProcessorEditor (*this);
}

//==============================================================================
void LowPassFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void LowPassFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LowPassFilterAudioProcessor();
}

// *****************************************************************************
// 2. Let's define the function createParameters()
juce::AudioProcessorValueTreeState::ParameterLayout LowPassFilterAudioProcessor::createParameters()
{ 
	std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
	parameters.push_back (std::make_unique<juce::AudioParameterFloat> ("FREQ", "CutOff Frequency", 50.0f, 20000.0f, 500.0f));
	parameters.push_back (std::make_unique<juce::AudioParameterFloat> ("Q", "Q Factor", 0.1f, 1.0f, 0.5f));
	return { parameters.begin(), parameters.end() };
}

