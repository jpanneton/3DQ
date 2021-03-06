//--------------------------------------------------------------------------------------------
// Name: OpenGLComponent.h
// Author: Jérémi Panneton
// Creation date: January 18th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "JuceHeader.h"
#include "Utilities/RingBuffer.h"
#include <memory>

//--------------------------------------------------------------------------------------------
/// OpenGL component with shader management and audio data access.
/// This is like a more elaborated version of the juce::OpenGLAppComponent class.
//--------------------------------------------------------------------------------------------
class OpenGLComponent : public Component, private OpenGLRenderer
{
protected:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] readSize                 Number of samples to read from the ring buffer before each render.
    /// @param[in] sampleRate               Sample rate.
    /// @param[in] continuousRepaint        True if OpenGL should render at a constant rate. False if OpenGL should render only on repaint event.
    //----------------------------------------------------------------------------------------
    OpenGLComponent(int readSize, double sampleRate, bool continuousRepaint);

    //----------------------------------------------------------------------------------------
    /// Destructor.
    //----------------------------------------------------------------------------------------
    virtual ~OpenGLComponent();

    //----------------------------------------------------------------------------------------
    /// Shuts down OpenGL.
    /// @warning This method must be called from the subclass destructor. Otherwise, render() will still be called in between subclass and base class destructors.
    /// @note This method should only be called once.
    //----------------------------------------------------------------------------------------
    void shutdownOpenGL();

protected:
    //----------------------------------------------------------------------------------------
    /// Implement this method to set up any GL objects that you need for rendering (i.e. create buffers).
    /// The GL context is active when this method is called.
    /// @warning This method is called automatically and should never be called explicitly.
    /// @note This method may be called more than once.
    //----------------------------------------------------------------------------------------
    virtual void initialise() = 0;

    //----------------------------------------------------------------------------------------
    /// Implement this method to compile shader code and create the actual shader program.
    //----------------------------------------------------------------------------------------
    virtual void createShaders() = 0;

    //----------------------------------------------------------------------------------------
    /// Implement this method to free any GL objects that you created for rendering (i.e. delete buffers).
    /// The GL context is still active when this method is called.
    /// @warning This method is called automatically and should never be called explicitly.
    /// @note This method may be called more than once.
    //----------------------------------------------------------------------------------------
    virtual void shutdown() {};

    //----------------------------------------------------------------------------------------
    /// Implement this method to render stuff on the screen.
    /// Usually, this is done by setting uniform variables, reading audio data and pushing vertices to the buffers.
    /// @warning This method is called automatically and should never be called explicitly.
    /// @see OpenGLRenderer::render()
    //----------------------------------------------------------------------------------------
    virtual void render() = 0;

public:
    //----------------------------------------------------------------------------------------
    /// Sets automatic rendering (constant rate).
    //----------------------------------------------------------------------------------------
    void start() noexcept;

    //----------------------------------------------------------------------------------------
    /// Sets manual rendering (on repaint event).
    /// This usually stops rendering since user needs to explicitly call OpenGLContext::triggerRepaint().
    //----------------------------------------------------------------------------------------
    void stop() noexcept;

    //----------------------------------------------------------------------------------------
    /// Called during playback to add the incoming audio blocks to the ring buffer.
    /// @param[in] buffer					Incoming audio buffer.
    //----------------------------------------------------------------------------------------
    void processBlock(const AudioBuffer<float>& buffer);

    //----------------------------------------------------------------------------------------
    /// Returns the number of samples to read from the ring buffer before each render.
    /// @return                             Number of samples to read from the ring buffer before each render.
    //----------------------------------------------------------------------------------------
    int getReadSize() const noexcept;

protected:
    //----------------------------------------------------------------------------------------
    /// Holds uniform variables of the shader program.
    /// @warning Subclass needs to be named Uniforms in order for the getUniforms() macro to work!
    //----------------------------------------------------------------------------------------
    struct ShaderUniforms
    {
        using Uniform = OpenGLShaderProgram::Uniform;
        virtual ~ShaderUniforms() = default;
    };

    //----------------------------------------------------------------------------------------
    /// This macro should be used in a subclass to get access to its uniform variables.
    //----------------------------------------------------------------------------------------
    #define getUniforms() static_cast<Uniforms*>(m_uniforms.get())

    OpenGLContext m_openGLContext;					/// OpenGL context.
    std::unique_ptr<OpenGLShaderProgram> m_shader;	/// Shader program.
    std::unique_ptr<ShaderUniforms> m_uniforms;		/// Shader program's uniform variables.
    Colour m_backgroundColor;						/// Color used when clearing the viewport.

    RingBuffer<float> m_ringBuffer;		/// Ring buffer that holds the incoming audio data.
    AudioBuffer<float> m_readBuffer;	/// Temporary buffer to store the latest ring buffer's audio frame.
    const double m_sampleRate = 0.0;    /// Sample rate.

    unsigned int m_fps = 0;             /// Number of frames rendered per second.

private:
    //----------------------------------------------------------------------------------------
    /// @see OpenGLRenderer::newOpenGLContextCreated.
    //----------------------------------------------------------------------------------------
    void newOpenGLContextCreated() override;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLRenderer::renderOpenGL.
    //----------------------------------------------------------------------------------------
    void renderOpenGL() override;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLRenderer::openGLContextClosing.
    //----------------------------------------------------------------------------------------
    void openGLContextClosing() override;

    double m_lastTimePoint = {};        /// Last time point (in ms).
    double m_ellapsedTime = {};         /// Elapsed time since last time point (in ms).
    unsigned int m_fpsCounter = 0;      /// FPS counter used to update m_fps each second.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLComponent)
};