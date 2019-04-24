//--------------------------------------------------------------------------------------------
// Name: Spectrogram3D.cpp
// Author: Jérémi Panneton
// Creation date: February 5th, 2019
//--------------------------------------------------------------------------------------------

#include "Spectrogram3D.h"
#include "StatusBar.h"
#include "DSP/Filters.h"
#include <numeric>

Spectrogram3D::Spectrogram3D(double sampleRate, StatusBar& statusBar)
    : Spectrogram(sampleRate, 256, statusBar)
    , m_spectrogramImage(Image::ARGB, m_frequencyAxis.getResolution(), m_frequencyAxis.getResolution(), false)
    , m_draggableOrientation(11.0f)
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
    m_xFreqWidth = 3.0f;
    m_yAmpHeight = 1.0f;
    m_zTimeDepth = 3.0f;

    // Time resolution = frequency resolution
    m_zTimeResolution = static_cast<GLuint>(m_frequencyAxis.getResolution());

    // Generate data
    initializeVertices();
    initializeIndices();

    // Generate buffers
    m_openGLContext.extensions.glGenVertexArrays(1, &m_VAO); // Vertex Array Object
    m_openGLContext.extensions.glGenBuffers(1, &m_VBO); // Vertex Buffer Object
    m_openGLContext.extensions.glGenBuffers(1, &m_EBO); // Element Buffer Object

    // Bind vertex array
    m_openGLContext.extensions.glBindVertexArray(m_VAO);

    // Bind vertex buffer
    m_openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    m_openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    // Bind element buffer
    m_openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    m_openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);

    // Define how the data should be pushed to the vertex shader
    m_openGLContext.extensions.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    m_openGLContext.extensions.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Vector3D<GLfloat>));
    m_openGLContext.extensions.glEnableVertexAttribArray(0);
    m_openGLContext.extensions.glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);

    m_spectrogramImage.clear(m_spectrogramImage.getBounds(), Colours::transparentBlack);
}

void Spectrogram3D::shutdown()
{
    // Free buffers
    m_openGLContext.extensions.glDeleteVertexArrays(1, &m_VAO);
    m_openGLContext.extensions.glDeleteBuffers(1, &m_VBO);
    m_openGLContext.extensions.glDeleteBuffers(1, &m_EBO);

    // Clear data
    m_vertices.clear();
    m_indices.clear();

    m_spectrogramTexture.release();
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

void Spectrogram3D::render()
{
    updateData();

    const int rightHandEdge = m_spectrogramImage.getWidth() - 1;
    m_spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, m_spectrogramImage.getHeight());

    // Calculate new y values and shift old y values back
    for (int y = 0; y < m_frequencyAxis.getResolution(); ++y)
    {
        const int j = m_frequencyAxis.getResolution() - y - 1;
        const auto frequencyInfo = getFrequencyInfo(y);
        const auto color = m_colorMap.getColorAtPosition(frequencyInfo.normalizedLevel);
        // Alpha channel is used for height. If the level exceeds 1.0, it gets clipped
        const auto texelValue = Colour::fromFloatRGBA(color.x, color.y, color.z, frequencyInfo.normalizedLevel);
        m_spectrogramImage.setPixelAt(rightHandEdge, j, texelValue);
    }

    m_spectrogramTexture.loadImage(m_spectrogramImage);

    Matrix3D<float> scale;
    scale.mat[0] = 2.0f;
    scale.mat[5] = 2.0f;
    scale.mat[10] = 2.0f;
    const Matrix3D<float> viewMatrix = scale * m_draggableOrientation.getViewMatrix();

    getUniforms()->projectionMatrix.setMatrix4(getProjectionMatrix().mat, 1, false);
    getUniforms()->viewMatrix.setMatrix4(viewMatrix.mat, 1, false);

    // GL_TEXTURE0 is activated by default
    m_spectrogramTexture.bind();
    m_openGLContext.extensions.glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    m_openGLContext.extensions.glBindVertexArray(0);
    m_spectrogramTexture.unbind();
}

void Spectrogram3D::resized()
{
    m_draggableOrientation.setViewport(getLocalBounds());
}

void Spectrogram3D::mouseDown(const MouseEvent& e)
{
    m_draggableOrientation.mouseDown(e.getPosition());
}

void Spectrogram3D::mouseDrag(const MouseEvent& e)
{
    m_draggableOrientation.mouseDrag(e.getPosition());
}

void Spectrogram3D::initializeVertices()
{
    const GLuint xFreqResolution = static_cast<GLuint>(m_frequencyAxis.getResolution());
    m_vertices.reserve((xFreqResolution + 2) * (m_zTimeResolution + 2));

    const GLfloat xOffset = m_xFreqWidth / m_frequencyAxis.getResolution();
    const GLfloat zOffset = m_zTimeDepth / m_zTimeResolution;
    const GLfloat xStart = +(m_xFreqWidth / 2.0f);
    const GLfloat zStart = -(m_zTimeDepth / 2.0f);

    const auto addFreqAtTime = [&](GLuint xFreqIndex, GLuint zTimeIndex, bool ignoreLevel)
    {
        Vertex vertex;
        vertex.position = { xStart - xFreqIndex * xOffset, 0.0f, zStart + zTimeIndex * zOffset };
        vertex.uv[0] = static_cast<GLfloat>(xFreqIndex) / (xFreqResolution - 1);
        vertex.uv[1] = static_cast<GLfloat>(zTimeIndex) / (m_zTimeResolution - 1);
        vertex.uv[2] = ignoreLevel ? 0.0f : 1.0f;
        m_vertices.push_back(std::move(vertex));
    };

    const auto addFreqRow = [&](GLuint zTimeIndex, bool ignoreLevel)
    {
        addFreqAtTime(0, zTimeIndex, true);
        for (GLuint xFreqIndex = 0; xFreqIndex < xFreqResolution; ++xFreqIndex)
        {
            addFreqAtTime(xFreqIndex, zTimeIndex, ignoreLevel);
        }
        addFreqAtTime(xFreqResolution - 1, zTimeIndex, true);
    };

    // Generates a grid of (xFreqResolution + 2) * (zTimeResolution + 2) vertices
    // The middle xFreqResolution * zTimeResolution grid is the actual data and
    // the additional +2 is to add a border at level 0 around the grid so that
    // the surface gets closed on all four sides
    addFreqRow(0, true);
    for (GLuint zTimeIndex = 0; zTimeIndex < m_zTimeResolution; ++zTimeIndex)
    {
        addFreqRow(zTimeIndex, false);
    }
    addFreqRow(m_zTimeResolution - 1, true);
}

void Spectrogram3D::initializeIndices()
{
    const GLuint xFreqResolution = static_cast<GLuint>(m_frequencyAxis.getResolution()) + 2;
    const GLuint zTimeResolution = m_zTimeResolution + 2;
    m_indices.reserve(6 * (xFreqResolution - 1) * (zTimeResolution - 1));
    
    for (GLuint zTimeIndex = 0; zTimeIndex < zTimeResolution - 1; ++zTimeIndex)
    {
        for (GLuint xFreqIndex = 0; xFreqIndex < xFreqResolution - 1; ++xFreqIndex)
        {
            const GLuint index = zTimeIndex * xFreqResolution + xFreqIndex;
            m_indices.push_back(index);
            m_indices.push_back(index + 1);
            m_indices.push_back(index + xFreqResolution);
            m_indices.push_back(index + 1);
            m_indices.push_back(index + xFreqResolution + 1);
            m_indices.push_back(index + xFreqResolution);
        }
    }
}

Matrix3D<float> Spectrogram3D::getProjectionMatrix() const noexcept
{
    float w = 1.0f / (0.5f + 0.1f);
    float h = w * getLocalBounds().toFloat().getAspectRatio(false);
    return Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
}