//--------------------------------------------------------------------------------------------
// Name: Spectrogram2D.cpp
// Author: Jérémi Panneton
// Creation date: February 2nd, 2019
//--------------------------------------------------------------------------------------------

#include "Spectrogram2D.h"
#include "StatusBar.h"
#include "DSP/Filters.h"
#include <numeric>

Spectrogram2D::Spectrogram2D(double sampleRate, StatusBar& statusBar)
    : Spectrogram(sampleRate, 512, statusBar)
    , m_spectrogramImage(Image::RGB, m_frequencyAxis.getResolution(), m_frequencyAxis.getResolution(), false)
{
    m_backgroundColor = Colour::fromRGB(25, 25, 25);
}

Spectrogram2D::~Spectrogram2D()
{
    // Turn off OpenGL
    shutdownOpenGL();
}

void Spectrogram2D::initialise()
{
    const Rectangle<int> bounds = m_spectrogramImage.getBounds();
    const GLshort left = static_cast<GLshort>(bounds.getX());
    const GLshort top = static_cast<GLshort>(bounds.getY());
    const GLshort right = static_cast<GLshort>(bounds.getRight());
    const GLshort bottom = static_cast<GLshort>(bounds.getBottom());
    const GLshort vertices[] = { left, bottom, right, bottom, left, top, right, top };

    // Generate buffers
    m_openGLContext.extensions.glGenVertexArrays(1, &m_VAO);
    m_openGLContext.extensions.glGenBuffers(1, &m_VBO);

    // Bind vertex array
    m_openGLContext.extensions.glBindVertexArray(m_VAO);

    // Bind vertex buffer
    m_openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    m_openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Define how the data should be pushed to the vertex shader
    m_openGLContext.extensions.glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 4, nullptr);
    m_openGLContext.extensions.glEnableVertexAttribArray(0);

    m_spectrogramImage.clear(m_spectrogramImage.getBounds(), Colours::black);
}

void Spectrogram2D::shutdown()
{
    // Free buffers
    m_openGLContext.extensions.glDeleteVertexArrays(1, &m_VAO);
    m_openGLContext.extensions.glDeleteBuffers(1, &m_VBO);
    
    // Clear data
    m_spectrogramTexture.release();
}

void Spectrogram2D::createShaders()
{
    constexpr char* vertexShader =
#include "Shaders/Spectrogram2D.vert"
        ;

    constexpr char* fragmentShader =
#include "Shaders/Spectrogram2D.frag"
        ;

    auto newShader = std::make_unique<OpenGLShaderProgram>(m_openGLContext);
    
    if (newShader->addVertexShader(vertexShader) &&
        newShader->addFragmentShader(fragmentShader) &&
        newShader->link())
    {
        m_shader = std::move(newShader);
        m_shader->use();
        m_uniforms = std::make_unique<Uniforms>(*m_shader);
    }
    else
    {
        jassertfalse;
    }
}

void Spectrogram2D::render()
{
    updateData();

    const int rightHandEdge = m_spectrogramImage.getWidth() - 1;
    m_spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, m_spectrogramImage.getHeight());

    // Calculate new y values and shift old y values back
    for (int y = 0; y < m_frequencyAxis.getResolution(); ++y)
    {
        const int j = m_frequencyAxis.getResolution() - y - 1;
        const auto frequencyInfo = getFrequencyInfo(y);
        const auto pixelColor = m_colorMap.getColorAtPosition(frequencyInfo.normalizedLevel);
        m_spectrogramImage.setPixelAt(rightHandEdge, j, Colour::fromFloatRGBA(pixelColor.x, pixelColor.y, pixelColor.z, 1.0f));
    }

    if (m_isMouseHover)
    {
        jassert(m_mousePosition.y < m_frequencyAxis.getResolution());
        const auto hoveredFrequencyInfo = getFrequencyInfo(m_mousePosition.y);
        m_statusBar.update(m_fps, hoveredFrequencyInfo.frequency, hoveredFrequencyInfo.dbLevel);
    }
    else
    {
        m_statusBar.update(m_fps, 0.0f, 0.0f);
    }
    
    const Rectangle<int> bounds = m_spectrogramImage.getBounds();
    m_spectrogramTexture.loadImage(m_spectrogramImage);
    
    const GLfloat textureBounds[] = {
        static_cast<GLfloat>(bounds.getX()),	 // Left
        static_cast<GLfloat>(bounds.getY()),	 // Top
        static_cast<GLfloat>(bounds.getWidth()), // Right
        static_cast<GLfloat>(bounds.getHeight()) // Bottom
    };

    getUniforms()->screenSize.set(textureBounds[2], textureBounds[3]);
    getUniforms()->textureBounds.set(textureBounds, 4);

    // GL_TEXTURE0 is activated by default
    m_spectrogramTexture.bind();
    m_openGLContext.extensions.glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_openGLContext.extensions.glBindVertexArray(0);
    m_spectrogramTexture.unbind();
}