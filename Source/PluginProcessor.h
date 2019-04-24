//--------------------------------------------------------------------------------------------
// Name: PluginProcessor.h
// Author: Jérémi Panneton
// Creation date: January 31st, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "GUI/MainComponent.h"

//--------------------------------------------------------------------------------------------
/// Audio thread of the plugin.
//--------------------------------------------------------------------------------------------
class PluginProcessor : public AudioProcessor
{
public:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    //----------------------------------------------------------------------------------------
    PluginProcessor();

    //----------------------------------------------------------------------------------------
    /// Destructor.
    //----------------------------------------------------------------------------------------
    ~PluginProcessor();

    //----------------------------------------------------------------------------------------
    /// @see AudioProcessor::prepareToPlay.
    //----------------------------------------------------------------------------------------
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    //----------------------------------------------------------------------------------------
    /// @see AudioProcessor::releaseResources.
    //----------------------------------------------------------------------------------------
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    //----------------------------------------------------------------------------------------
    /// @see AudioProcessor::processBlock.
    //----------------------------------------------------------------------------------------
    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;

    //==============================================================================
    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
private:
    friend class PluginEditor;
    MainComponent m_visualizer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};