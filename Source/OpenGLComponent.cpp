//--------------------------------------------------------------------------------------------
// Name: OpenGLComponent.cpp
// Author: J�r�mi Panneton
// Creation date: January 18th, 2019
//--------------------------------------------------------------------------------------------

#include "OpenGLComponent.h"

OpenGLComponent::OpenGLComponent(RingBuffer<GLfloat>& ringBuffer, int readSize, double sampleRate, bool continuousRepaint)
    : m_backgroundColor(getLookAndFeel().findColour(ResizableWindow::backgroundColourId))
    , m_ringBuffer(ringBuffer)
	, m_readBuffer(2, readSize)
    , m_sampleRate(sampleRate)
{
	m_openGLContext.setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);
	m_openGLContext.setRenderer(this);
	m_openGLContext.attachTo(*this);
    //openGLContext.setComponentPaintingEnabled(false);
	m_openGLContext.setContinuousRepainting(continuousRepaint);
}

OpenGLComponent::~OpenGLComponent()
{
    // Before your subclass's destructor has completed, you must call
    // shutdownOpenGL() to release the GL context. (Otherwise there's
    // a danger that it may invoke a GL callback on your class while
    // it's in the process of being deleted.
    jassert(!m_openGLContext.isAttached());
    shutdownOpenGL();
}

void OpenGLComponent::start()
{
	m_openGLContext.setContinuousRepainting(true);
}

void OpenGLComponent::stop()
{
	m_openGLContext.setContinuousRepainting(false);
}

int OpenGLComponent::getReadSize() const
{
    return m_readBuffer.getNumSamples();
}

void OpenGLComponent::shutdownOpenGL()
{
	m_openGLContext.setContinuousRepainting(false);
	m_openGLContext.detach();
}

void OpenGLComponent::newOpenGLContextCreated()
{
    initialise();
    createShaders();
}

void OpenGLComponent::renderOpenGL()
{
    jassert(OpenGLHelpers::isContextActive());

    // Setup Viewport
    const float renderingScale = (float)m_openGLContext.getRenderingScale();
    glViewport(0, 0, roundToInt(renderingScale * getWidth()), roundToInt(renderingScale * getHeight()));

    // Set background color
    OpenGLHelpers::clear(m_backgroundColor);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Use Shader Program that's been defined
	m_shader->use();

    render();
}

void OpenGLComponent::openGLContextClosing()
{
    shutdown();
	m_shader->release();
	m_shader = nullptr;
	m_uniforms = nullptr;
}