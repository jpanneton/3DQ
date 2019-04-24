//--------------------------------------------------------------------------------------------
// Name: OpenGLComponent.cpp
// Author: Jérémi Panneton
// Creation date: January 18th, 2019
//--------------------------------------------------------------------------------------------

#include "OpenGLComponent.h"

OpenGLComponent::OpenGLComponent(int readSize, double sampleRate, bool continuousRepaint)
    : m_backgroundColor(getLookAndFeel().findColour(ResizableWindow::backgroundColourId))
    , m_ringBuffer(2, readSize * 10)
    , m_readBuffer(2, readSize)
    , m_sampleRate(sampleRate)
{
    m_readBuffer.clear();

    m_openGLContext.setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);
    m_openGLContext.setComponentPaintingEnabled(false);
    m_openGLContext.setContinuousRepainting(continuousRepaint);
    m_openGLContext.setRenderer(this);
    m_openGLContext.attachTo(*this);
}

OpenGLComponent::~OpenGLComponent()
{
    // Before the subclass's destructor has completed, shutdownOpenGL()
    // must be called to release the GL context. Otherwise, there's a
    // danger that it may invoke a GL callback on the subclass while
    // it's in the process of being deleted.
    jassert(!m_openGLContext.isAttached());
    shutdownOpenGL();
}

void OpenGLComponent::start() noexcept
{
    m_ringBuffer.clear();
    m_openGLContext.setContinuousRepainting(true);
}

void OpenGLComponent::stop() noexcept
{
    m_openGLContext.setContinuousRepainting(false);
    m_ringBuffer.clear();
}

void OpenGLComponent::processBlock(const AudioBuffer<float>& buffer)
{
    m_ringBuffer.writeSamples(buffer);
}

int OpenGLComponent::getReadSize() const noexcept
{
    return m_readBuffer.getNumSamples();
}

void OpenGLComponent::shutdownOpenGL()
{
    m_openGLContext.setContinuousRepainting(false);
    m_openGLContext.detach();
    m_ringBuffer.clear();
}

void OpenGLComponent::newOpenGLContextCreated()
{
    initialise();
    createShaders();
}

void OpenGLComponent::renderOpenGL()
{
    jassert(m_openGLContext.isActive());

    // Setup viewport
    const double renderingScale = m_openGLContext.getRenderingScale();
    glViewport(0, 0, roundToInt(renderingScale * getWidth()), roundToInt(renderingScale * getHeight()));

    // Set background color
    OpenGLHelpers::clear(m_backgroundColor);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Use shader program that's been defined
    m_shader->use();

    // Render component
    render();

    // Update statistics
    const double currentTimePoint = Time::getMillisecondCounterHiRes();
    m_ellapsedTime += (currentTimePoint - m_lastTimePoint);
    m_lastTimePoint = currentTimePoint;
    ++m_fpsCounter;

    if (m_ellapsedTime >= 1000.0) // 1 second
    {
        m_ellapsedTime = 0.0;
        m_fps = m_fpsCounter;
        m_fpsCounter = 0;
    }
}

void OpenGLComponent::openGLContextClosing()
{
    shutdown();
    m_shader->release();
    m_shader = nullptr;
    m_uniforms = nullptr;
}