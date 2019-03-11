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
    /// @param sampleRate                   Sample rate.
    //----------------------------------------------------------------------------------------
    Spectrogram3D(double sampleRate);

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
	/// @see OpenGLComponent::shutdown.
	//----------------------------------------------------------------------------------------
	void shutdown() override;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::render.
    //----------------------------------------------------------------------------------------
    void render() override;

	//----------------------------------------------------------------------------------------
	/// @see OpenGLComponent::resized.
	//----------------------------------------------------------------------------------------
	void resized() override;

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
        Uniforms(OpenGLShaderProgram& shaderProgram)
			: imageTexture(shaderProgram, "imageTexture")
			, projectionMatrix(shaderProgram, "projectionMatrix")
			, viewMatrix(shaderProgram, "viewMatrix")
        {
        }

        Uniform imageTexture, projectionMatrix, viewMatrix;
    };

    GLfloat m_xFreqWidth;				/// Frequency axis size.
    GLfloat m_yAmpHeight;				/// Amplitude axis size.
    GLfloat m_zTimeDepth;				/// Time axis size.
	GLuint m_zTimeResolution;			/// Time axis resolution.

	struct Vertex
	{
		Vector3D<GLfloat> position;		/// 3D position of the vertex.
		GLfloat uv[3];					/// UV coordinates and a level scale factor.
	};

    std::vector<Vertex> m_vertices;		/// Vertices used by OpenGL.
    std::vector<GLuint> m_indices;		/// Indices used by OpenGL.

	Image m_spectrogramImage;			/// Sliding spectrogram image (CPU).
	OpenGLTexture m_spectrogramTexture;	/// Generated texture from the sliding spectrogram image (GPU).

	GLuint m_VAO, m_VBO, m_EBO;			/// OpenGL buffers ID.

    // GUI Interaction
    DraggableOrbitCamera m_draggableOrientation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrogram3D)
};