//--------------------------------------------------------------------------------------------
// Name: MainComponent.cpp
// Author: Jérémi Panneton
// Creation date: February 2nd, 2019
//--------------------------------------------------------------------------------------------

#include "MainComponent.h"
#include "Utilities/ColorGradients.h"
#include "Visualizers/Spectrogram2D.h"
#include "Visualizers/Spectrogram3D.h"

const Colour MainComponent::BACKGROUND_COLOR(0, 0, 0);
const Colour MainComponent::SEPARATOR_COLOR(125, 125, 125);

MainComponent::MainComponent()
{
    // Setup GUI
    addAndMakeVisible(m_statusBar);
    addAndMakeVisible(m_controlPanel);

    const auto addButton = [&](Button& button, const char* text, bool toggled)
    {
        m_controlPanel.addAndMakeVisible(button);
        button.setButtonText(text);
        button.setColour(TextButton::buttonColourId, Colour(0xFF0C4B95));
        button.onClick = [&] { buttonClicked(&button); };
        button.setToggleState(toggled, NotificationType::dontSendNotification);
    };

    addButton(m_spectrogram2DButton, "Spectrogram 2D", false);
    addButton(m_spectrogram3DButton, "Spectrogram 3D", false);
    addButton(m_lowFrequencyButton, "Low Frequency Mode", false);
    addButton(m_adaptiveLevelButton, "Adaptive Level", false);
    addButton(m_clipLevelButton, "Clip Level", false);
}

MainComponent::~MainComponent()
{
}

void MainComponent::prepareToPlay(double sampleRate)
{
    // Create visualizers
    m_spectrogram2D = std::make_unique<Spectrogram2D>(sampleRate, m_statusBar);
    addChildComponent(m_spectrogram2D.get());

    m_spectrogram3D = std::make_unique<Spectrogram3D>(sampleRate, m_statusBar);
    addChildComponent(m_spectrogram3D.get());
}

void MainComponent::releaseResources()
{
    if (m_spectrogram2D)
    {
        m_spectrogram2D->stop();
        removeChildComponent(m_spectrogram2D.get());
        m_spectrogram2D = nullptr;
    }

    if (m_spectrogram3D)
    {
        m_spectrogram3D->stop();
        removeChildComponent(m_spectrogram3D.get());
        m_spectrogram3D = nullptr;
    }
}

void MainComponent::processBlock(AudioBuffer<float>& buffer)
{
    if (m_activeVisualizer)
    {
        m_activeVisualizer->processBlock(buffer);
    }
}

void MainComponent::paint(Graphics& g)
{
    g.fillAll(BACKGROUND_COLOR);
    g.setColour(SEPARATOR_COLOR);
    g.drawHorizontalLine(m_statusBar.getY() + 1, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(m_controlPanel.getY() + 1, 0.0f, static_cast<float>(getWidth()));
}

void MainComponent::resized()
{
    // Main component
    const int width = getWidth();
    const int height = getHeight();
    const int statusBarY = static_cast<int>(VISUALIZER_RATIO * height);
    const int controlPanelY = statusBarY + CONTROL_HEIGHT;

    m_statusBar.setBounds(0, statusBarY, width, CONTROL_HEIGHT);
    m_controlPanel.setBounds(0, controlPanelY, width, height - controlPanelY);
    
    // Controls
    constexpr int panelPadding = 20;

    // Buttons
    const int buttonWidth = (width - 30) / 2;
    constexpr int buttonHeight = 20;
    constexpr int buttonMargin = 10;

    m_spectrogram2DButton.setBounds(panelPadding, CONTROL_HEIGHT, buttonWidth, buttonHeight);
    m_spectrogram3DButton.setBounds(panelPadding, CONTROL_HEIGHT * 2, buttonWidth, buttonHeight);
    m_lowFrequencyButton.setBounds(panelPadding, CONTROL_HEIGHT * 3, buttonWidth, buttonHeight);
    m_adaptiveLevelButton.setBounds(panelPadding, CONTROL_HEIGHT * 4, buttonWidth, buttonHeight);
    m_clipLevelButton.setBounds(panelPadding, CONTROL_HEIGHT * 5, buttonWidth, buttonHeight);

    if (m_spectrogram2D)
        m_spectrogram2D->setBounds(0, 0, width, statusBarY);
    if (m_spectrogram3D)
        m_spectrogram3D->setBounds(0, 0, width, statusBarY);
}

void MainComponent::buttonClicked(Button* button)
{
    const bool buttonToggleState = button->getToggleState();

    if (button == &m_spectrogram2DButton)
    {
        m_spectrogram3DButton.setToggleState(false, NotificationType::dontSendNotification);

        m_spectrogram3D->setVisible(false);
        m_spectrogram2D->setVisible(buttonToggleState);

        m_spectrogram3D->stop();
        m_spectrogram2D->stop();

        m_activeVisualizer = m_spectrogram2D.get();
        m_activeVisualizer->start();
        resized();
    }
    else if (button == &m_spectrogram3DButton)
    {
        m_spectrogram2DButton.setToggleState(false, NotificationType::dontSendNotification);
        
        m_spectrogram2D->setVisible(false);
        m_spectrogram3D->setVisible(buttonToggleState);

        m_spectrogram2D->stop();
        m_spectrogram3D->stop();

        m_activeVisualizer = m_spectrogram3D.get();
        m_activeVisualizer->start();
        resized();
    }
    else if (button == &m_lowFrequencyButton)
    {
        const ColourGradient gradient = !buttonToggleState ? ColorGradients::getDefaultGradient() : ColorGradients::getRedGradient();
        m_spectrogram2D->setMaxFrequency(!buttonToggleState ? 44100.0f / 2.0f : 200.0f, gradient);
        m_spectrogram3D->setMaxFrequency(!buttonToggleState ? 44100.0f / 2.0f : 200.0f, gradient);
    }
    else if (button == &m_adaptiveLevelButton)
    {
        m_spectrogram2D->setAdaptiveLevel(buttonToggleState);
        m_spectrogram3D->setAdaptiveLevel(buttonToggleState);
    }
    else if (button == &m_clipLevelButton)
    {
        m_spectrogram2D->setClipLevel(buttonToggleState);
        m_spectrogram3D->setClipLevel(buttonToggleState);
    }
}