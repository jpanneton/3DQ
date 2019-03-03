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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(700, 700);
    addAndMakeVisible(m_processor.m_visualizer);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint(Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	m_processor.m_visualizer.setBounds(getLocalBounds());
}