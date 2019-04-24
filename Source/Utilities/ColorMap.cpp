#include "ColorMap.h" 
 
void ColorMap::setGradient(const ColourGradient& gradient) 
{ 
    // A valid gradient has at least two color stops 
    jassert(gradient.getNumColours() > 1); 
    m_colorGradient = gradient; 
    remapColors(); 
} 
 
const ColorMap::PixelRGB* ColorMap::getData() const noexcept 
{ 
    return m_colorMap.data(); 
} 
 
ColorMap::PixelRGB ColorMap::operator[](size_t index) const 
{ 
    return m_colorMap[index]; 
} 
 
ColorMap::PixelRGB ColorMap::colorToVector(const Colour& color) 
{ 
    return { color.getFloatRed(), color.getFloatGreen(), color.getFloatBlue() }; 
} 
 
void ColorMap::remapColors() 
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