//--------------------------------------------------------------------------------------------
// Name: PluginEditor.cpp
// Author: Jérémi Panneton
// Creation date: January 31st, 2019
//--------------------------------------------------------------------------------------------

#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor& processor)
    : AudioProcessorEditor(&processor)
    , m_processor(processor)
{
    // Size needs to be set before construction!
    setSize(700, 700);
    addAndMakeVisible(m_processor.m_visualizer);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint(Graphics& g)
{
    g.fillAll(Colours::black);
}

void PluginEditor::resized()
{
    constexpr int padding = 3;
    m_processor.m_visualizer.setBounds(getLocalBounds().reduced(padding));
}