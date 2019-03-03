//--------------------------------------------------------------------------------------------
// Name: Spectrogram3D.h
// Author: Jérémi Panneton
// Creation date: February 5th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "Spectrogram.h"
#include "Utilities/DraggableOrbitCamera.h"
#include <vector>

//--------------------------------------------------------------------------------------------
/// 3D spectrogram visualizer. Handles mouse interaction and real-time display.
/// The points are calculated on the CPU, but the color mapping is perform on the GPU (fragment shader).
//--------------------------------------------------------------------------------------------
class Spectrogram3D : public Spectrogram
{
public:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param ringBuffer                   Reference to the audio thread's ring buffer.
    /// @param sampleRate                   Sample rate.
    //----------------------------------------------------------------------------------------
    Spectrogram3D(RingBuffer<GLfloat>& ringBuffer, double sampleRate);

    //----------------------------------------------------------------------------------------
    /// Destructor.
    //----------------------------------------------------------------------------------------
    ~Spectrogram3D();

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

	//----------------------------------------------------------------------------------------
	/// @see OpenGLComponent::resized.
	//----------------------------------------------------------------------------------------
	void resized() override;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::shutdown.
    //----------------------------------------------------------------------------------------
    void shutdown() override;

    //----------------------------------------------------------------------------------------
    /// @see Component::mouseDown.
    //----------------------------------------------------------------------------------------
    void mouseDown(const MouseEvent& e) override;

    //----------------------------------------------------------------------------------------
    /// @see Component::mouseDrag.
    //----------------------------------------------------------------------------------------
    void mouseDrag(const MouseEvent& e) override;

private:
    //==========================================================================
    // Mesh Functions

	//----------------------------------------------------------------------------------------
	/// Creates a grid of vertices.
	//----------------------------------------------------------------------------------------
    void initializeVertices();

	//----------------------------------------------------------------------------------------
	/// Generates the indices needed to form a surface from the vertex grid.
	//----------------------------------------------------------------------------------------
    void initializeIndices();

    //==========================================================================
    // OpenGL Functions

	//----------------------------------------------------------------------------------------
	/// Calculates a projection matrix based on the window size and a default frustum.
	/// @return								Projection matrix.
	//----------------------------------------------------------------------------------------
    Matrix3D<float> getProjectionMatrix() const noexcept;

private:
    //----------------------------------------------------------------------------------------
    /// Holds uniform variables of the shader program.
    //----------------------------------------------------------------------------------------
    struct Uniforms : public ShaderUniforms
    {
        Uniforms(OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
        {
            projectionMatrix = createUniform(openGLContext, shaderProgram, "projectionMatrix");
            viewMatrix = createUniform(openGLContext, shaderProgram, "viewMatrix");
        }

        UniformPtr projectionMatrix, viewMatrix;
    };

    GLfloat xFreqWidth;					/// Frequency axis size.
    GLfloat yAmpHeight;					/// Amplitude axis size.
    GLfloat zTimeDepth;					/// Time axis size.
	GLuint zTimeResolution;				/// Time axis resolution.

    using Vertex = Vector3D<GLfloat>;
    std::vector<Vertex> vertices;		/// Vertices used by OpenGL.
    std::vector<GLuint> indices;		/// Indices used by OpenGL.

	GLuint VAO, VBO, EBO;				/// OpenGL buffers ID.

    // GUI Interaction
    DraggableOrbitCamera draggableOrientation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrogram3D)
};