//--------------------------------------------------------------------------------------------
// Name: Spectrogram2D.cpp
// Author: Jérémi Panneton
// Creation date: February 2nd, 2019
//--------------------------------------------------------------------------------------------

#include "Spectrogram2D.h"
#include "DSP/Filters.h"
#include <numeric>

Spectrogram2D::Spectrogram2D(double sampleRate)
    : Spectrogram(sampleRate, 512)
    , m_spectrogramImage(Image::RGB, m_frequencyAxis.getResolution(), m_frequencyAxis.getResolution(), true)
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
    String statusText;

    if (newShader->addVertexShader(vertexShader) &&
		newShader->addFragmentShader(fragmentShader) &&
		newShader->link())
    {
		m_shader = std::move(newShader);
		m_shader->use();

        statusText = "GLSL: v" + String(OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else
    {
        statusText = newShader->getLastError();
    }

    //statusLabel.setText(statusText, dontSendNotification);
}

void Spectrogram2D::render()
{
    updateData();

    const int rightHandEdge = m_spectrogramImage.getWidth() - 1;
	m_spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, m_spectrogramImage.getHeight());

    // Calculate new y values and shift old y values back
    for (int y = 0; y < m_frequencyAxis.getResolution(); ++y)
    {
		const auto frequencyInfo = getFrequencyInfo(y);

        if (m_isMouseHover && m_mousePosition.y == y)
        {
            const MessageManagerLock mmLock;
			m_statusLabel.setText(std::to_string(frequencyInfo.frequency), dontSendNotification);
        }

		const int j = m_frequencyAxis.getResolution() - y - 1;
		// Colour::fromHSV(level, 1.0f, level, 1.0f)
		// const Colour pixelColor = Colour::fromHSV((1.0f - frequencyInfo.level) * 0.3f, 1.0f, frequencyInfo.level, 1.0f);
		const auto pixelColor = m_colorMap.getColorAtPosition(1.0f - frequencyInfo.level);
		m_spectrogramImage.setPixelAt(rightHandEdge, j, Colour::fromFloatRGBA(pixelColor.x, pixelColor.y, pixelColor.z, 1.0f));
    }
    
    Rectangle<int> bounds(m_spectrogramImage.getWidth(), m_spectrogramImage.getHeight());
	m_spectrogramTexture.loadImage(m_spectrogramImage);
	m_spectrogramTexture.bind();
	m_openGLContext.copyTexture(bounds, bounds, bounds.getWidth(), bounds.getHeight(), false);
	m_spectrogramTexture.unbind();
}