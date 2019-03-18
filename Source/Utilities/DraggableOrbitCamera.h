//--------------------------------------------------------------------------------------------
// Name: DraggableOrbitCamera.h
// Author: Jérémi Panneton
// Creation date: February 26th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "JuceHeader.h"
#include "Math.h"

//--------------------------------------------------------------------------------------------
/// Generates orbital camera transformations from mouse input events and a set of parameters.
//--------------------------------------------------------------------------------------------
class DraggableOrbitCamera
{
public:
    using MatrixType = Matrix3D<float>;
    using VectorType = Vector3D<float>;
    using QuaternionType = Quaternion<float>;

    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] orbitalRadius			Distance from the center point (point where the camera is pointing).
    //----------------------------------------------------------------------------------------
    DraggableOrbitCamera(float orbitalRadius) noexcept
    {
        setOrbitalAxis(VectorType::yAxis());
        setOrbitalRadius(orbitalRadius);
    }

    //----------------------------------------------------------------------------------------
    /// Sets the frame of the camera.
    /// @param[in] newArea			        Viewport frame.
    //----------------------------------------------------------------------------------------
    void setViewport(Rectangle<int> newArea) noexcept
    {
        m_area = newArea;
    }

    //----------------------------------------------------------------------------------------
    /// Sets the rotation axis of the camera.
    /// @param[in] axis			            Rotation axis.
    //----------------------------------------------------------------------------------------
    void setOrbitalAxis(const VectorType& axis) noexcept
    {
        m_quaternion = m_baseQuaternion = QuaternionType(axis, 0);
    }

    //----------------------------------------------------------------------------------------
    /// Sets the distance from the center point.
    /// @param[in] radius					Distance between the point where the camera is pointing and the camera.
    //----------------------------------------------------------------------------------------
    void setOrbitalRadius(float radius) noexcept
    {
        m_radius = jmax(0.1f, radius);
    }

    //----------------------------------------------------------------------------------------
    /// Mouse down event callback.
    /// @param[in] mousePos					Current mouse position.
    //----------------------------------------------------------------------------------------
    void mouseDown(const Point<int>& mousePos) noexcept
    {
        m_lastMousePos = mousePosToProportion(mousePos.toFloat());
    }

    //----------------------------------------------------------------------------------------
    /// Mouse drag event callback.
    /// @param[in] mousePos					Current mouse position.
    //----------------------------------------------------------------------------------------
    void mouseDrag(const Point<int>& mousePos) noexcept
    {
        auto newMousePos = mousePosToProportion(mousePos.toFloat());

        m_pitch += (newMousePos.y - m_lastMousePos.y);
        m_yaw += (newMousePos.x - m_lastMousePos.x);

        if (m_pitch > 0.0f)
            m_pitch = 0.0f;
        if (m_pitch < -pi<float> / 2.0f)
            m_pitch = -pi<float> / 2.0f;

        QuaternionType pitchQuat = QuaternionType::fromAngle(m_pitch, VectorType::xAxis());
        QuaternionType yawQuat = QuaternionType::fromAngle(-m_yaw, VectorType::yAxis());
        yawQuat *= pitchQuat;
        m_quaternion = m_baseQuaternion;
        m_quaternion *= yawQuat.normalised();

        m_lastMousePos = newMousePos;
    }

    //----------------------------------------------------------------------------------------
    /// Returns the transformation matrix of the current camera state.
    /// @return								View matrix.
    //----------------------------------------------------------------------------------------
    MatrixType getViewMatrix() const noexcept
    {
        MatrixType viewMatrix(VectorType(0.0f, 0.0f, -m_radius));
        MatrixType rotationMatrix = m_quaternion.getRotationMatrix();
        return rotationMatrix * viewMatrix;
    }

private:
    //----------------------------------------------------------------------------------------
    /// Converts a screen position in pixels to a normalized position (between -1 and 1).
    /// Allows dragging to be relative to the viewport size.
    /// @param[in] mousePos					Mouse position in pixels.
    /// @return								Normalized mouse position.
    //----------------------------------------------------------------------------------------
    Point<float> mousePosToProportion(const Point<float>& mousePos) const noexcept
    {
        // setViewport() must be called before any mouse input callbacks!
        jassert(m_area.getWidth() > 0 && m_area.getHeight() > 0);

        const float scale = jmin(m_area.getWidth(), m_area.getHeight()) / 2.0f;
        return { (mousePos.x - m_area.getCentreX()) / scale,
            (m_area.getCentreY() - mousePos.y) / scale };
    }

    float m_pitch = {};					/// Elevation of the camera.
    float m_yaw = {};					/// Azimuth of the camera.
    float m_radius = {};				/// Distance of the camera from the center point.
    Rectangle<int> m_area;				/// Frame of the camera.
    QuaternionType m_baseQuaternion;	/// Rotation axis quaternion.
    QuaternionType m_quaternion;		/// Camera quaternion.
    Point<float> m_lastMousePos;		/// Last mouse drag position.
};