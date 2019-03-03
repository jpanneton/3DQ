//--------------------------------------------------------------------------------------------
// Name: Spectrogram3D.cpp
// Author: J�r�mi Panneton
// Creation date: February 5th, 2019
//--------------------------------------------------------------------------------------------

#include "Spectrogram3D.h"
#include "DSP/Filters.h"
#include <numeric>

Spectrogram3D::Spectrogram3D(RingBuffer<GLfloat>& ringBuffer, double sampleRate)
    : Spectrogram(ringBuffer, sampleRate, 250)
    , draggableOrientation(10.0f)
{
	m_backgroundColor = Colour::fromRGB(25, 25, 25);
}

Spectrogram3D::~Spectrogram3D()
{
    // Turn off OpenGL
    shutdownOpenGL();
}

void Spectrogram3D::initialise()
{
    // Set sizing parameters
    xFreqWidth = 3.0f;
    yAmpHeight = 1.0f;
    zTimeDepth = 3.0f;

	// Time resolution = frequency resolution
    zTimeResolution = static_cast<GLuint>(m_frequencyAxis.getResolution());

	// Generate data
    initializeVertices();
    initializeIndices();

	// Generate buffers
	m_openGLContext.extensions.glGenVertexArrays(1, &VAO); // Vertex Array Object
	m_openGLContext.extensions.glGenBuffers(1, &VBO); // Vertex Buffer Object
	m_openGLContext.extensions.glGenBuffers(1, &EBO); // Element Buffer Object

	// Bind vertex array
	m_openGLContext.extensions.glBindVertexArray(VAO);

	// Bind vertex buffer
	m_openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, VBO);
	m_openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

	// Bind element buffer
	m_openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	m_openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	// Define how the data should be pushed to the vertex shader
	m_openGLContext.extensions.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	m_openGLContext.extensions.glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
}

void Spectrogram3D::createShaders()
{
    constexpr char* vertexShader =
#include "Shaders/Spectrogram3D.vert"
        ;

	constexpr char* fragmentShader =
#include "Shaders/Spectrogram3D.frag"
        ;
	
    auto newShader = std::make_unique<OpenGLShaderProgram>(m_openGLContext);
    String statusText;

    if (newShader->addVertexShader(vertexShader) &&
		newShader->addFragmentShader(fragmentShader) &&
		newShader->link())
    {
		m_shader = std::move(newShader);
		m_shader->use();
		m_uniforms = std::make_unique<Uniforms>(m_openGLContext, *m_shader);

        statusText = "GLSL: v" + String(OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else
    {
        statusText = newShader->getLastError();
    }

    //statusLabel.setText(statusText, dontSendNotification);
}

void Spectrogram3D::render()
{
    updateData();

    // Calculate new y values and shift old y values back
    for (int i = static_cast<int>(vertices.size()); i --> 0;)
    {
        // For the first row of points, render the new height via the FFT
        if (i < m_frequencyAxis.getResolution())
        {
            vertices[i].y = getFrequencyInfo(i).level;
        }
        else // For the subsequent rows, shift back
        {
            vertices[i].y = vertices[i - m_frequencyAxis.getResolution()].y;
        }
    }
    
	m_openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, VBO);
	m_openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);

    // Setup the Uniforms for use in the Shader
    if (getUniforms()->projectionMatrix != nullptr)
        getUniforms()->projectionMatrix->setMatrix4(getProjectionMatrix().mat, 1, false);

    if (getUniforms()->viewMatrix != nullptr)
    {
        Matrix3D<float> scale;
        scale.mat[0] = 2.0f;
        scale.mat[5] = 2.0f;
        scale.mat[10] = 2.0f;
        Matrix3D<float> finalMatrix = scale * draggableOrientation.getViewMatrix();
        getUniforms()->viewMatrix->setMatrix4(finalMatrix.mat, 1, false);
    }

    // Draw the points
	m_openGLContext.extensions.glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	m_openGLContext.extensions.glBindVertexArray(0);
}

void Spectrogram3D::shutdown()
{
    vertices.clear();
    indices.clear();
}

void Spectrogram3D::resized()
{
    draggableOrientation.setViewport(getLocalBounds());
}

void Spectrogram3D::mouseDown(const MouseEvent& e)
{
    draggableOrientation.mouseDown(e.getPosition());
}

void Spectrogram3D::mouseDrag(const MouseEvent& e)
{
    draggableOrientation.mouseDrag(e.getPosition());
}

void Spectrogram3D::initializeVertices()
{
	const GLuint xFreqResolution = static_cast<GLuint>(m_frequencyAxis.getResolution());
    vertices.reserve(m_frequencyAxis.getResolution() * zTimeResolution);

    // Variables when setting x and z
    const GLfloat xOffset = xFreqWidth / m_frequencyAxis.getResolution();
    const GLfloat zOffset = zTimeDepth / zTimeResolution;
    const GLfloat xStart = -(xFreqWidth / 2.0f);
    const GLfloat zStart = -(zTimeDepth / 2.0f);

    // Set all X and Z values
    for (GLuint zTimeIndex = 0; zTimeIndex < zTimeResolution; ++zTimeIndex)
    {
        for (GLuint xFreqIndex = 0; xFreqIndex < xFreqResolution; ++xFreqIndex)
        {
            vertices.emplace_back(xStart + xFreqIndex * xOffset, 0.0f, zStart + zTimeIndex * zOffset);
        }
    }
}

void Spectrogram3D::initializeIndices()
{
	const GLuint xFreqResolution = static_cast<GLuint>(m_frequencyAxis.getResolution());
    indices.reserve(6 * (xFreqResolution - 1) * (zTimeResolution - 1));
    
    for (GLuint z = 0; z < zTimeResolution - 1; z++)
    {
        for (GLuint x = 0; x < xFreqResolution - 1; x++)
        {
            const GLuint index = z * xFreqResolution + x;
            indices.push_back(index);
            indices.push_back(index + 1);
            indices.push_back(index + xFreqResolution);
            indices.push_back(index + 1);
            indices.push_back(index + xFreqResolution + 1);
            indices.push_back(index + xFreqResolution);
        }
    }
}

Matrix3D<float> Spectrogram3D::getProjectionMatrix() const noexcept
{
    float w = 1.0f / (0.5f + 0.1f);
    float h = w * getLocalBounds().toFloat().getAspectRatio(false);
    return Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
}