//--------------------------------------------------------------------------------------------
// Name: StatusBar.cpp
// Author: Jérémi Panneton
// Creation date: April 20th, 2019
//--------------------------------------------------------------------------------------------

#include "StatusBar.h"

StatusBar::StatusBar()
{
    addAndMakeVisible(m_fpsLabel);
    addAndMakeVisible(m_frequencyLabel);
    addAndMakeVisible(m_levelLabel);
}

void StatusBar::update(unsigned int fps, float frequency, float level)
{
    // Parameters must be captured by copy! Otherwise, referenced parameters will be invalid at call time.
    MessageManager::callAsync([&, fps, frequency]
    {
        m_fpsLabel.setText("FPS: " + String(fps), NotificationType::dontSendNotification);
        m_frequencyLabel.setText("Frequency: " + String(frequency), NotificationType::dontSendNotification);
        m_levelLabel.setText("Level: " + String(static_cast<int>(level)), NotificationType::dontSendNotification);
    });
}

void StatusBar::resized()
{
    const int width = getWidth();
    const int height = getHeight();
    m_fpsLabel.setBounds(0, 0, width / 8, height);
    m_frequencyLabel.setBounds(width / 8, 0, width / 4, height);
    m_levelLabel.setBounds(width / 8 + width / 4, 0, width / 4, height);
}