//--------------------------------------------------------------------------------------------
// Name: ColorMap.h
// Author: Jérémi Panneton
// Creation date: March 4th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "JuceHeader.h"
#include <vector>

//--------------------------------------------------------------------------------------------
/// Wraps a discrete colormap. Used to bake a gradient and speed up interpolated color indexing.
//--------------------------------------------------------------------------------------------
class ColorMap
{
    using PixelRGB = Vector3D<float>;

public:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] resolution				Initial number of color variations the map should contain.
    //----------------------------------------------------------------------------------------
    template<typename SizeType>
    ColorMap(SizeType resolution)
        : m_colorMap(resolution)
    {
        m_colorGradient.addColour(0, Colours::black);
        m_colorGradient.addColour(1, Colours::black);
        remapColors();
    }

    //----------------------------------------------------------------------------------------
    /// Sets the color gradient used for mapping.
    /// @param[in] gradient					Color gradient.
    //----------------------------------------------------------------------------------------
    void setGradient(const ColourGradient& gradient)
    {
        // A valid gradient has at least two color stops
        jassert(gradient.getNumColours() > 1);
        m_colorGradient = gradient;
        remapColors();
    }

    //----------------------------------------------------------------------------------------
    /// Sets the resolution (or size) of the color map.
    /// @param[in] resolution				Number of color variations the map should contain.
    //----------------------------------------------------------------------------------------
    template<typename SizeType>
    void setResolution(SizeType resolution)
    {
        if (resolution != m_frequencyMap.size())
        {
            m_colorMap.resize(resolution);
            remapLevels();
        }
    }

    //----------------------------------------------------------------------------------------
    /// Returns the current resolution (or size) of the color map.
    /// @return								Number of color variations contained in the map.
    //----------------------------------------------------------------------------------------
    template<typename SizeType = int>
    SizeType getResolution() const noexcept
    {
        return static_cast<SizeType>(m_colorMap.size());
    }

    //----------------------------------------------------------------------------------------
    /// Returns the mapped color at the specified index.
    /// @param[in] index					Index of the color (discrete position on the color map).
    /// @return								Color at the specified index.
    //----------------------------------------------------------------------------------------
    PixelRGB operator[](size_t index) const
    {
        return m_colorMap[index];
    }

    //----------------------------------------------------------------------------------------
    /// Returns the interpolated color at the specified normalized position.
    /// @param[in] position					Normalized position in the color gradient (between 0 and 1). The position gets clipped if it is out of bounds.
    /// @return								Interpolated color at the specified position.
    //----------------------------------------------------------------------------------------
    template<typename ValueType>
    PixelRGB getColorAtPosition(ValueType position) const
    {
        static_assert(std::is_floating_point_v<ValueType>);
        position = jlimit(ValueType(0), ValueType(1), position);
        // Instead of using position * (m_colorMap.size() - 1) with the original position
        // to get a valid index, we get the position right before so that it's never
        // exactly 1.0 and use m_colorMap.size(). By doing that, the last color maps to
        // the same position range as the other colors (otherwise it would only be 1.0).
        position = std::nextafter(position, position - ValueType(1));
        return m_colorMap[static_cast<size_t>(position * m_colorMap.size())];
    }

private:
    //----------------------------------------------------------------------------------------
    /// Converts a JUCE color to a floating point vector.
    /// @param[in] color					JUCE color.
    /// @return								Color in floating point vector format.
    //----------------------------------------------------------------------------------------
    PixelRGB colorToVector(const Colour& color)
    {
        return { color.getFloatRed(), color.getFloatGreen(), color.getFloatBlue() };
    }

    //----------------------------------------------------------------------------------------
    /// Maps the colors defined in the gradient using interpolation.
    //----------------------------------------------------------------------------------------
    void remapColors()
    {
        const size_t colorCount = m_colorMap.size();
        jassert(colorCount > 0);

        m_colorMap.front() = colorToVector(m_colorGradient.getColourAtPosition(0));
        m_colorMap.back() = colorToVector(m_colorGradient.getColourAtPosition(1));

        const double stepSize = 1.0 / (colorCount - 1);

        for (size_t i = 1; i < colorCount - 1; ++i)
        {
            m_colorMap[i] = colorToVector(m_colorGradient.getColourAtPosition(i * stepSize));
        }
    }

    std::vector<PixelRGB> m_colorMap;	/// Color mapping.
    ColourGradient m_colorGradient;		/// Color gradient.
};