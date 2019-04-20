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
        gradient.addColour(0.0, Colours::white);
        gradient.addColour(0.25, Colours::yellow);
        gradient.addColour(0.50, Colours::red);
        gradient.addColour(0.75, Colours::blue);
        gradient.addColour(1.0, Colours::black);
        return gradient;
    }

    inline ColourGradient getRedGradient()
    {
        ColourGradient gradient;
        gradient.addColour(0.0, Colours::white);
        gradient.addColour(0.15, Colours::red);
        gradient.addColour(0.25, Colour(200, 0, 0));
        gradient.addColour(0.50, Colour(50, 0, 0));
        gradient.addColour(0.75, Colours::blue);
        gradient.addColour(1.0, Colours::black);
        return gradient;
    }
}