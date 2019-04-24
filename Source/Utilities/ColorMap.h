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
    using Container = std::vector<PixelRGB>;

public:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] resolution               Initial number of color variations the map should contain.
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
    /// @param[in] gradient                 Color gradient.
    //----------------------------------------------------------------------------------------
    void setGradient(const ColourGradient& gradient);

    //----------------------------------------------------------------------------------------
    /// Sets the resolution (or size) of the color map.
    /// @param[in] resolution               Number of color variations the map should contain.
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
    /// @return                             Number of color variations contained in the map.
    //----------------------------------------------------------------------------------------
    template<typename SizeType = int>
    SizeType getResolution() const noexcept
    {
        return static_cast<SizeType>(m_colorMap.size());
    }

    //----------------------------------------------------------------------------------------
    /// Returns the raw color map data.
    /// @return                             Array of colors.
    //----------------------------------------------------------------------------------------
    const PixelRGB* getData() const noexcept;

    //----------------------------------------------------------------------------------------
    /// Returns the mapped color at the specified index.
    /// @param[in] index                    Index of the color (discrete position on the color map).
    /// @return                             Color at the specified index.
    //----------------------------------------------------------------------------------------
    PixelRGB operator[](size_t index) const;

    //----------------------------------------------------------------------------------------
    /// Returns the interpolated color at the specified normalized position.
    /// @param[in] position                 Normalized position in the color gradient (between 0 and 1). The position gets clipped if it is out of bounds.
    /// @return                             Interpolated color at the specified position.
    //----------------------------------------------------------------------------------------
    template<typename ValueType>
    PixelRGB getColorAtPosition(ValueType position) const
    {
        static_assert(std::is_floating_point_v<ValueType>);
        position = jlimit(ValueType(0), ValueType(1), position);
        // Instead of using position * (m_colorMap.size() - 1) with the original position
        // to get a valid index, we calculate the index from the full size to allow the
        // last color to have an equal chance of being chosen. In fact, since position gets
        // floored to an integer, only a position of exactly 1.0 would map to the last color
        // otherwise.
        size_t index = static_cast<size_t>(position * m_colorMap.size());
        // If the position was exactly 1.0, we decrement the index to prevent overflow
        if (index == m_colorMap.size())
            --index;
        return m_colorMap[index];
    }

private:
    //----------------------------------------------------------------------------------------
    /// Converts a JUCE color to a floating point vector.
    /// @param[in] color                    JUCE color.
    /// @return                             Color in floating point vector format.
    //----------------------------------------------------------------------------------------
    PixelRGB colorToVector(const Colour& color);

    //----------------------------------------------------------------------------------------
    /// Maps the colors defined in the gradient using interpolation.
    //----------------------------------------------------------------------------------------
    void remapColors();

    Container m_colorMap;               /// Color mapping.
    ColourGradient m_colorGradient;     /// Color gradient.
};