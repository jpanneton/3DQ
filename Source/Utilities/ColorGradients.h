//--------------------------------------------------------------------------------------------
// Name: ColorGradients.h
// Author: Jérémi Panneton
// Creation date: April 6th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "JuceHeader.h"

namespace ColorGradients
{
    inline ColourGradient getDefaultGradient()
    {
        ColourGradient gradient;
        gradient.addColour(0.0, Colours::black);
        gradient.addColour(0.25, Colours::blue);
        gradient.addColour(0.50, Colours::red);
        gradient.addColour(0.75, Colours::yellow);
        gradient.addColour(1.0, Colours::white);
        return gradient;
    }

    inline ColourGradient getRedGradient()
    {
        ColourGradient gradient;
        gradient.addColour(0.0, Colours::black);
        gradient.addColour(0.25, Colours::blue);
        gradient.addColour(0.50, Colour(50, 0, 0));
        gradient.addColour(0.75, Colour(200, 0, 0));
        gradient.addColour(0.85, Colours::red);
        gradient.addColour(1.0, Colours::white);
        return gradient;
    }
}