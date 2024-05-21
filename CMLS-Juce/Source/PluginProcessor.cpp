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
                       ), F1(), F2(), F3()
{
        if (!connect(8000)) {
        std::cerr << "Unable to connect to port 8000\n";
       // juce::OSCReceiver::addListerner(this, "/cicco");
        }else std::printf("connected to port 8000\n");
        juce::OSCReceiver::addListener(this, "/handMovement/x");
        juce::OSCReceiver::addListener(this, "/handMovement/y");
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
    s.numChannels = getTotalNumInputChannels();
    s.sampleRate = sampleRate;

    F1.prepare(s);
    F2.prepare(s);
    F3.prepare(s);
    F1.reset();
    F2.reset();
    F3.reset();


    for(int i = 0; i < 3; i++){
        juce::AudioBuffer<float> l(1, samplesPerBlock);
        juce::AudioBuffer<float> r(1, samplesPerBlock);
        L_bands.push_back(std::move(l));
        R_bands.push_back(std::move(r));
    }
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

void CMLSJuceAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
    
    F1.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    F2.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    F3.setType(juce::dsp::StateVariableTPTFilterType::bandpass);

    juce::AudioBuffer<float> copy1 = buffer;
    juce::AudioBuffer<float> copy2 = buffer;
    juce::AudioBuffer<float> copy3 = buffer;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> block1(copy1);
    juce::dsp::AudioBlock<float> block2(copy2);
    juce::dsp::AudioBlock<float> block3(copy3);

    juce::dsp::ProcessContextReplacing<float> ctx1(block1);
    juce::dsp::ProcessContextReplacing<float> ctx2(block2);
    juce::dsp::ProcessContextReplacing<float> ctx3(block3);

    std::vector<float> mags;
    for (int i = 0; i < totalNumOutputChannels; i++) {
        mags.push_back(buffer.getMagnitude(i,0,buffer.getNumSamples()));
        block.getSingleChannelBlock(i).multiplyBy(1/mags[i]);
    }
    float freqs[3];
    float gains[3];
    float Qs[3];

    freqs[0] = *tree_state.getRawParameterValue("F1_Freq"); 
    gains[0] =  std::pow(10,0.1*(*tree_state.getRawParameterValue("F1_Gain")));
    Qs[0] = *tree_state.getRawParameterValue("F1_Q");

    freqs[1] = *tree_state.getRawParameterValue("F2_Freq"); 
    gains[1] = std::pow(10,0.1*(*tree_state.getRawParameterValue("F2_Gain")));
    Qs[1] = *tree_state.getRawParameterValue("F2_Q");

    freqs[2] = *tree_state.getRawParameterValue("F3_Freq"); 
    gains[2] =  std::pow(10,0.1*(*tree_state.getRawParameterValue("F3_Gain")));
    Qs[2] = *tree_state.getRawParameterValue("F3_Q");

    calcFreqs();

    F1.setCutoffFrequency(freq1);
    F2.setCutoffFrequency(freq2);
    F3.setCutoffFrequency(freq3);

    F1.setResonance(Qs[0]);
    F2.setResonance(Qs[1]);
    F3.setResonance(Qs[2]);

    F1.process(ctx1);
    F2.process(ctx2);
    F3.process(ctx3);

    for(int i = 0; i < totalNumOutputChannels; i++){
        auto writer = buffer.getWritePointer(i);
        for (int j = 0; j < buffer.getNumSamples(); j++)
            writer[j] = (mags[i])*(gains[0]*block1.getSample(i,j) + gains[1]*block2.getSample(i,j) + gains[2]*block3.getSample(i,j));
    }
}

//==============================================================================
bool CMLSJuceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CMLSJuceAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
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
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Freq", "Formant_1_Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 500.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Freq", "Formant_2_Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 1000.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Freq", "Formant_3_Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 2000.f)); 
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Gain", "Formant_1_Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), 0.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Gain", "Formant_2_Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), -6.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Gain", "Formant_3_Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), -6.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Q", "Formant_1_Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.05f, 1.f), 0.9f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Q", "Formant_2 Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.05f, 1.f), 0.9f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Q", "Formant_3_Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.05f, 1.f), 0.9f));
    return {layout.begin(), layout.end()};
}

void CMLSJuceAudioProcessor::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.size() == 1 && message[0].isInt32()) {
        if (message.getAddressPattern() == "/handMovement/x") {
            midi[0] = message[0].getInt32();
            std::printf("value obtined = %i\n", message[0].getInt32());
        } else if (message.getAddressPattern() == "/handMovement/y") {
            midi[1] = message[0].getInt32();
            std::cout << "message obtained = " << message[0].getInt32() << std::endl;
        }
    }
}

void CMLSJuceAudioProcessor::calcFreqs(){
    freq1 = (midi[0]/127.f)*f1_band + f1_min;
    freq2 = (midi[1]/127.f)*f2_band + f2_min;
    freq3 = 0; // qualcosa
}