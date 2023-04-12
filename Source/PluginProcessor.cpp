/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FeatheringIdeaStartAudioProcessor::FeatheringIdeaStartAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

FeatheringIdeaStartAudioProcessor::~FeatheringIdeaStartAudioProcessor()
{
}

//==============================================================================
const juce::String FeatheringIdeaStartAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FeatheringIdeaStartAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FeatheringIdeaStartAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FeatheringIdeaStartAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FeatheringIdeaStartAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FeatheringIdeaStartAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FeatheringIdeaStartAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FeatheringIdeaStartAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FeatheringIdeaStartAudioProcessor::getProgramName (int index)
{
    return {};
}

void FeatheringIdeaStartAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FeatheringIdeaStartAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;

    spec.numChannels = 1;

    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    auto chainSettings = getChainSettings(apvts);

    auto peak1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peak1Freq, chainSettings.peak1Quality,
        juce::Decibels::decibelsToGain(chainSettings.peak1GainInDecibels));

    auto peak2Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peak2Freq, chainSettings.peak2Quality,
        juce::Decibels::decibelsToGain(chainSettings.peak2GainInDecibels));

    auto peak3Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peak3Freq, chainSettings.peak3Quality,
        juce::Decibels::decibelsToGain(chainSettings.peak3GainInDecibels));

    auto peak4Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peak4Freq, chainSettings.peak4Quality,
        juce::Decibels::decibelsToGain(chainSettings.peak4GainInDecibels));
    

    

    *leftChain.get<ChainPositions::Peak1>().coefficients = *peak1Coefficients;
    *leftChain.get<ChainPositions::Peak2>().coefficients = *peak2Coefficients;
    *leftChain.get<ChainPositions::Peak3>().coefficients = *peak3Coefficients;
    *leftChain.get<ChainPositions::Peak4>().coefficients = *peak4Coefficients;

    *rightChain.get<ChainPositions::Peak1>().coefficients = *peak1Coefficients;
    *rightChain.get<ChainPositions::Peak2>().coefficients = *peak2Coefficients;
    *rightChain.get<ChainPositions::Peak3>().coefficients = *peak3Coefficients;
    *rightChain.get<ChainPositions::Peak4>().coefficients = *peak4Coefficients;
    
}

void FeatheringIdeaStartAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FeatheringIdeaStartAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FeatheringIdeaStartAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    auto chainSettings = getChainSettings(apvts);

    auto peak1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peak1Freq, chainSettings.peak1Quality,
        juce::Decibels::decibelsToGain(chainSettings.peak1GainInDecibels));

    auto peak2Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peak2Freq, chainSettings.peak2Quality,
        juce::Decibels::decibelsToGain(chainSettings.peak2GainInDecibels));

    auto peak3Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peak3Freq, chainSettings.peak3Quality,
        juce::Decibels::decibelsToGain(chainSettings.peak3GainInDecibels));

    auto peak4Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peak4Freq, chainSettings.peak4Quality,
        juce::Decibels::decibelsToGain(chainSettings.peak4GainInDecibels));




    *leftChain.get<ChainPositions::Peak1>().coefficients = *peak1Coefficients;
    *leftChain.get<ChainPositions::Peak2>().coefficients = *peak2Coefficients;
    *leftChain.get<ChainPositions::Peak3>().coefficients = *peak3Coefficients;
    *leftChain.get<ChainPositions::Peak4>().coefficients = *peak4Coefficients;

    *rightChain.get<ChainPositions::Peak1>().coefficients = *peak1Coefficients;
    *rightChain.get<ChainPositions::Peak2>().coefficients = *peak2Coefficients;
    *rightChain.get<ChainPositions::Peak3>().coefficients = *peak3Coefficients;
    *rightChain.get<ChainPositions::Peak4>().coefficients = *peak4Coefficients;


    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool FeatheringIdeaStartAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FeatheringIdeaStartAudioProcessor::createEditor()
{
    //return new FeatheringIdeaStartAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void FeatheringIdeaStartAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FeatheringIdeaStartAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.peak1Freq = apvts.getRawParameterValue("Freq 1")->load();
    settings.peak2Freq = apvts.getRawParameterValue("Freq 2")->load();
    settings.peak3Freq = apvts.getRawParameterValue("Freq 3")->load();
    settings.peak4Freq = apvts.getRawParameterValue("Freq 4")->load();

    settings.peak1GainInDecibels = apvts.getRawParameterValue("Gain 1")->load();
    settings.peak2GainInDecibels = apvts.getRawParameterValue("Gain 2")->load();
    settings.peak3GainInDecibels = apvts.getRawParameterValue("Gain 3")->load();
    settings.peak4GainInDecibels = apvts.getRawParameterValue("Gain 4")->load();

    settings.peak1Quality = apvts.getRawParameterValue("Q 1")->load();
    settings.peak2Quality = apvts.getRawParameterValue("Q 2")->load();
    settings.peak3Quality = apvts.getRawParameterValue("Q 3")->load();
    settings.peak4Quality = apvts.getRawParameterValue("Q 4")->load();

    return settings;
}

//STARTING WITH 4 Bands to Feather
juce::AudioProcessorValueTreeState::ParameterLayout FeatheringIdeaStartAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("Freq 1",
                                                           "Freq 1",
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                            250.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Gain 1",
                                                           "Gain 1",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Q 1",
                                                           "Q 1",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Freq 2",
                                                           "Freq 2",
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                            500.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Gain 2",
                                                           "Gain 2",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Q 2",
                                                           "Q 2",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Freq 3",
                                                           "Freq 3",
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                            750.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Gain 3",
                                                           "Gain 3",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Q 3",
                                                           "Q 3",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Freq 4",
                                                           "Freq 4",
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                            1000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Gain 4",
                                                           "Gain 4",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Q 4",
                                                           "Q 4",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FeatheringIdeaStartAudioProcessor();
}
