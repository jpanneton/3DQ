//--------------------------------------------------------------------------------------------
// Name: Spectrogram2D.h
// Author: Jérémi Panneton
// Creation date: February 2nd, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "Spectrogram.h"

//--------------------------------------------------------------------------------------------
/// Standard spectrogram visualizer.
/// The color mapping is perform on the CPU and then applied as a texture on the GPU.
//--------------------------------------------------------------------------------------------
class Spectrogram2D : public Spectrogram
{
public:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param ringBuffer                   Reference to the audio thread's ring buffer.
    /// @param sampleRate                   Sample rate.
    //----------------------------------------------------------------------------------------
    Spectrogram2D(RingBuffer<GLfloat>& ringBuffer, double sampleRate);

    //----------------------------------------------------------------------------------------
    /// Destructor.
    //----------------------------------------------------------------------------------------
    ~Spectrogram2D();

protected:
    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::initialise.
    //----------------------------------------------------------------------------------------
    void initialise() override;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::createShaders.
    //----------------------------------------------------------------------------------------
    void createShaders() override;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::render.
    //----------------------------------------------------------------------------------------
    void render() override;

private:
    Image m_spectrogramImage;			/// Sliding spectrogram image (CPU).
    OpenGLTexture m_spectrogramTexture;	/// Generated texture from the sliding spectrogram image (GPU).

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrogram2D)
};