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
        if (!connect(8000)) {
        std::cerr << "Unable to connect to port 8000\n";
       // juce::OSCReceiver::addListerner(this, "/cicco");
        juce::OSCReceiver::addListener(this, "/handMovement/*");
    }
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
    s.numChannels = 1;
    s.sampleRate = sampleRate;

    L_Peak.prepare(s);
    L_Peak.reset();
    R_Peak.prepare(s);
    R_Peak.reset();

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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    juce::dsp::AudioBlock<float> block(buffer);
    // std::vector<juce::dsp::ProcessContextReplacing<float>> L_ctxs, R_ctxs;
    juce::AudioBuffer<float> Lb(1,buffer.getNumSamples());
    juce::AudioBuffer<float> Rb(1,buffer.getNumSamples());
    juce::dsp::AudioBlock<float> L_out(Lb), R_out(Rb);

    // --------- From here
    float L_mag = buffer.getMagnitude(0,0,buffer.getNumSamples());
    float R_mag = buffer.getMagnitude(1,0,buffer.getNumSamples());
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

    for(int i = 0; i < L_bands.size(); i++){
        L_bands[i].clear(0,L_bands[i].getNumSamples());
        R_bands[i].clear(0,L_bands[i].getNumSamples());

        L_bands[i].copyFrom(0, 0, block.getChannelPointer(0), L_bands[i].getNumSamples());
        R_bands[i].copyFrom(0, 0, block.getChannelPointer(1), R_bands[i].getNumSamples());
        
        juce::dsp::AudioBlock<float> L_block(L_bands[i]);
        juce::dsp::AudioBlock<float> R_block(R_bands[i]);
        juce::dsp::ProcessContextReplacing<float> L_ctx(L_block);
        juce::dsp::ProcessContextReplacing<float> R_ctx(R_block);
        //L_ctxs.push_back(std::move(L_ctx));
        //R_ctxs.push_back(std::move(R_ctx));

        L_Peak.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(Fs, freqs[i], Qs[i], gains[i]/L_mag);
        L_Peak.process(L_ctx);
        R_Peak.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(Fs, freqs[i], Qs[i], gains[i]/R_mag);
        R_Peak.process(R_ctx);
        L_out.add(L_ctx.getOutputBlock());
        R_out.add(R_ctx.getOutputBlock());
    }
    
    auto L = block.getSingleChannelBlock(0);
    auto R = block.getSingleChannelBlock(1);

    L.swap(L_out);
    R.swap(R_out);
    
    //L_ctxs.clear();
    //R_ctxs.clear();
    // float l_Mval = 0;
    // float r_Mval = 0;
    // for(int j = 0; j < block.getNumSamples(); j++){
    //     float l_val = 0;
    //     float r_val = 0;
    //     for(int i = 0; i < L_bands.size(); i++){
    //         l_val += L_bands[i].getSample(0, j);
    //         r_val += R_bands[i].getSample(0,j);
    //     }

    //     if(abs(l_val) > l_Mval) l_Mval = abs(l_val);
    //     if(abs(r_val) > r_Mval) r_Mval = abs(r_val);

    //     buffer.setSample(0, j, l_val);
    //     buffer.setSample(1, j, r_val);
    // }
    // block.getSingleChannelBlock(0).multiplyBy(L_mag/l_Mval);
    // block.getSingleChannelBlock(1).multiplyBy(R_mag/r_Mval);   
    // --------- Up To here

    // juce::dsp::ProcessContextReplacing<float> L_ctx(L_block);
    // juce::dsp::ProcessContextReplacing<float> R_ctx(R_block);

    // float in_magnitude = buffer.getMagnitude(0,buffer.getNumSamples());
    // float curr_freq1 = f1_min + (f1_band*midi[0])/127;
    // float curr_freq2 = f2_min + (f2_band*midi[1])/127;

    // juce::AudioBuffer<float> copy1;
    // copy1.makeCopyOf<float>(buffer,1); 
    // juce::AudioBuffer<float> copy2;
    // copy2.makeCopyOf<float>(buffer,1); 
    // juce::AudioBuffer<float> copy3;
    // copy3.makeCopyOf<float>(buffer,1); 
    // //juce::AudioBuffer<float> empty { buffer.getNumChannels(), buffer.getNumSamples() };

    // juce::dsp::AudioBlock<float> block1(copy1);
    // juce::dsp::AudioBlock<float> block2(copy2);
    // juce::dsp::AudioBlock<float> block3(copy3);
    
    // juce::dsp::AudioBlock<float> out(buffer);
    // //juce::dsp::AudioBlock<float> app(empty);

    // if(F1_freq != curr_freq1) {
    //     F1_freq = curr_freq1;
    //     tree_state.state.setProperty("F1_Freq", F1_freq, nullptr);
    // }
    // if(F2_freq != curr_freq2) {
    //     F2_freq = curr_freq2;
    //     tree_state.state.setProperty("F2_Freq", F2_freq, nullptr);
    // }
    //Formant_1.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,2000,1);

    // Formant_2.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,1000,1);

    // Formant_3.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,500,1);

    // // out.multiplyBy(0.1);

    // auto context = juce::dsp::ProcessContextReplacing<float>(out);

    // Formant_1.process(context);
    // Formant_2.process(context);
    // Formant_3.process(context);
    // Formant_1.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,F1_freq,F1_q);
    // Formant_2.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,F2_freq,F2_q);
    // Formant_3.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(Fs,F3_freq,F3_q);

    // block1.multiplyBy(1/in_magnitude);
    // block2.multiplyBy(1/in_magnitude);
    // block3.multiplyBy(1/in_magnitude);

    // auto context1 = juce::dsp::ProcessContextReplacing<float>(block1);
    // auto context2 = juce::dsp::ProcessContextReplacing<float>(block2);
    // auto context3 = juce::dsp::ProcessContextReplacing<float>(block3);

    // Formant_1.process(context1);
    // Formant_2.process(context2);
    // Formant_3.process(context3);
    
    // context1.getOutputBlock().multiplyBy(F1_gain);
    // context2.getOutputBlock().multiplyBy(F2_gain);
    // context3.getOutputBlock().multiplyBy(F3_gain);

    // for (int j; j < out.getNumChannels(); j++)
    // for (int i = 0; i < out.getNumSamples(); i++) {
    //    out.setSample(j, i, context1.getOutputBlock().getSample(j,i) + context2.getOutputBlock().getSample(j,i) + context3.getOutputBlock().getSample(j,i));
    // }
    // float out_magnitude = buffer.getMagnitude(0,buffer.getNumSamples());
    // out.multiplyBy(in_magnitude/out_magnitude);
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
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Freq", "Formant 1 Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 500.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Freq", "Formant 2 Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 1000.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Freq", "Formant 3 Freq", juce::NormalisableRange<float>(20.f, 20000.f,1.f, 1.f), 2000.f)); 
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Gain", "Formant 1 Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), 0.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Gain", "Formant 2 Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), -6.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Gain", "Formant 3 Gain", juce::NormalisableRange<float>(-24.f, 0.f,1.f, 1.f), -6.f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F1_Q", "Formant 1 Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.05f, 1.f), 0.1f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F2_Q", "Formant 2 Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.05f, 1.f), 0.1f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("F3_Q", "Formant 3 Qfact", juce::NormalisableRange<float>(0.1f, 10.f,0.05f, 1.f), 0.1f));
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
