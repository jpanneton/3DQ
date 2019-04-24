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
    /// @param[in] sampleRate               Sample rate.
    /// @param[out] statusBar               Reference to the status bar (GUI).
    //----------------------------------------------------------------------------------------
    Spectrogram2D(double sampleRate, StatusBar& statusBar);

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
    /// @see OpenGLComponent::shutdown.
    //----------------------------------------------------------------------------------------
    void shutdown() override;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::render.
    //----------------------------------------------------------------------------------------
    void render() override;

private:
    Image m_spectrogramImage;			/// Sliding spectrogram image (CPU).
    OpenGLTexture m_spectrogramTexture;	/// Generated texture from the sliding spectrogram image (GPU).

    GLuint m_VAO, m_VBO;				/// OpenGL buffers ID.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrogram2D)
};