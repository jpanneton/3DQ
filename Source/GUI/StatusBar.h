//--------------------------------------------------------------------------------------------
// Name: StatusBar.h
// Author: Jérémi Panneton
// Creation date: April 20th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "JuceHeader.h"

//--------------------------------------------------------------------------------------------
/// Status bar component. Handles the update of the UI from the rendering thread.
//--------------------------------------------------------------------------------------------
class StatusBar : public Component
{
public:
    //----------------------------------------------------------------------------------------
    /// Default constructor.
    //----------------------------------------------------------------------------------------
    StatusBar();

    //----------------------------------------------------------------------------------------
    /// Updates the displayed values on the status bar.
    /// @param[in] fps				        Current FPS of the visualizer.
    /// @param[in] frequency				Frequency currently hovered by mouse.
    /// @param[in] level				    Level in dB of the frequency hovered by the mouse.
    //----------------------------------------------------------------------------------------
    void update(unsigned int fps, float frequency, float level);

    //----------------------------------------------------------------------------------------
    /// Resizes UI elements according to the status bar size (JUCE, not OpenGL).
    //----------------------------------------------------------------------------------------
    void resized() override;

private:
    Label m_fpsLabel;           /// Current FPS of the visualizer.
    Label m_frequencyLabel;     /// Frequency currently hovered by mouse.
    Label m_levelLabel;         /// Level in dB of the frequency hovered by the mouse.
};