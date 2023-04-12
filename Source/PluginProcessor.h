/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct ChainSettings
{
    float peak1Freq{ 0 }, peak1GainInDecibels{ 0 }, peak1Quality{ 1.f };
    float peak2Freq{ 0 }, peak2GainInDecibels{ 0 }, peak2Quality{ 1.f };
    float peak3Freq{ 0 }, peak3GainInDecibels{ 0 }, peak3Quality{ 1.f };
    float peak4Freq{ 0 }, peak4GainInDecibels{ 0 }, peak4Quality{ 1.f };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class FeatheringIdeaStartAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    FeatheringIdeaStartAudioProcessor();
    ~FeatheringIdeaStartAudioProcessor() override;

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

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:

    using Filter = juce::dsp::IIR::Filter<float>;
    
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
                                                //was CutFilter, Filter, Cutfilter
    using MonoChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    MonoChain leftChain, rightChain;

    enum ChainPositions
    {
        Peak1,
        Peak2,
        Peak3,
        Peak4

    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeatheringIdeaStartAudioProcessor)
};
