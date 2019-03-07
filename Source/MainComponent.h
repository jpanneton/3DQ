//--------------------------------------------------------------------------------------------
// Name: MainComponent.h
// Author: J�r�mi Panneton
// Creation date: February 2nd, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "JuceHeader.h"
#include "Utilities/RingBuffer.h"
#include <memory>

class OpenGLComponent;
class Spectrogram2D;
class Spectrogram3D;

//--------------------------------------------------------------------------------------------
/// Main component of the editor. Contains all the UI related stuff.
/// Based on https://github.com/TimArt/3DAudioVisualizers.
//--------------------------------------------------------------------------------------------
class MainComponent : public Component, public Button::Listener
{
public:
	//----------------------------------------------------------------------------------------
	/// Default constructor. Adds controls to the component and sets its initial size.
	//----------------------------------------------------------------------------------------
	MainComponent();

	//----------------------------------------------------------------------------------------
	/// Destructor.
	//----------------------------------------------------------------------------------------
	~MainComponent();

	//========================================================================================
	// Audio callbacks (processor)

	//----------------------------------------------------------------------------------------
	/// Called before playback starts to allocate the resources and initialize the UI elements.
	/// @param[in] sampleRate				Sample rate (fixed during playback).
	//----------------------------------------------------------------------------------------
	void prepareToPlay(double sampleRate);

	//----------------------------------------------------------------------------------------
	/// Called after playback has stopped to free the resources and close the UI elements.
	//----------------------------------------------------------------------------------------
	void releaseResources();

	//----------------------------------------------------------------------------------------
	/// Called during playback to process the incoming audio blocks.
	/// @param[in,out] buffer				Audio buffer to process.
	//----------------------------------------------------------------------------------------
	void processBlock(AudioBuffer<float>& buffer);

	//========================================================================================
	// GUI callbacks (editor)

	//----------------------------------------------------------------------------------------
	/// Paints UI elements and graphics on screen (JUCE, not OpenGL). Called before OpenGL rendering.
	//----------------------------------------------------------------------------------------
	void paint(Graphics& g) override;

	//----------------------------------------------------------------------------------------
	/// Resizes UI elements according to the main window size (JUCE, not OpenGL).
	//----------------------------------------------------------------------------------------
	void resized() override;

	//----------------------------------------------------------------------------------------
	/// Called when a button is clicked.
	/// @param[in] button					Button being clicked.
	//----------------------------------------------------------------------------------------
	void buttonClicked(Button* button) override;

private:
	Image m_background;

	// GUI buttons
	TextButton m_spectrogram2DButton;
	TextButton m_spectrogram3DButton;

	// Audio buffer
	std::unique_ptr<RingBuffer<float>> m_ringBuffer;

	// Visualizers
	std::unique_ptr<Spectrogram2D> m_spectrogram2D;
	std::unique_ptr<Spectrogram3D> m_spectrogram3D;

	OpenGLComponent* m_activeVisualizer = nullptr;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};