//--------------------------------------------------------------------------------------------
// Name: MainComponent.cpp
// Author: Jérémi Panneton
// Creation date: February 2nd, 2019
//--------------------------------------------------------------------------------------------

#include "MainComponent.h"
#include "Visualizers/Spectrogram2D.h"
#include "Visualizers/Spectrogram3D.h"

MainComponent::MainComponent()
{
    // Setup GUI
    addAndMakeVisible(&m_spectrogram2DButton);
    m_spectrogram2DButton.setButtonText("Spectrogram 2D");
    m_spectrogram2DButton.setColour(TextButton::buttonColourId, Colour(0xFF0C4B95));
    m_spectrogram2DButton.addListener(this);
    m_spectrogram2DButton.setToggleState(false, NotificationType::dontSendNotification);

    addAndMakeVisible(&m_spectrogram3DButton);
    m_spectrogram3DButton.setButtonText("Spectrogram 3D");
    m_spectrogram3DButton.setColour(TextButton::buttonColourId, Colour(0xFF0C4B95));
    m_spectrogram3DButton.addListener(this);
    m_spectrogram3DButton.setToggleState(false, NotificationType::dontSendNotification);

	m_background = ImageCache::getFromMemory(BinaryData::Background_png, BinaryData::Background_pngSize);
}

MainComponent::~MainComponent()
{
}

void MainComponent::prepareToPlay(double sampleRate)
{
    // Create visualizers
	m_spectrogram2D = std::make_unique<Spectrogram2D>(sampleRate);
    addChildComponent(m_spectrogram2D.get());

	m_spectrogram3D = std::make_unique<Spectrogram3D>(sampleRate);
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
    g.fillAll(Colour(0xFF252831));
    g.drawImageAt(m_background, 0, 0);
}

void MainComponent::resized()
{
    const int border = 3;
    const int w = getWidth();
    const int h = getHeight();

    // Button Dimenstions
    const int bWidth = (w - 30) / 2;
    const int bHeight = 20;
    const int bMargin = 10;

	m_spectrogram2DButton.setBounds(bWidth + 2 * bMargin, h - 80, bWidth, bHeight);
	m_spectrogram3DButton.setBounds(bWidth + 2 * bMargin, h - 40, bWidth, bHeight);

    if (m_spectrogram2D != nullptr)
		m_spectrogram2D->setBounds(border, border, w - 2 * border, 509 - border);
    if (m_spectrogram3D != nullptr)
		m_spectrogram3D->setBounds(border, border, w - 2 * border, 509 - border);
}

void MainComponent::buttonClicked(Button* button)
{
    if (button == &m_spectrogram2DButton)
    {
        bool buttonToggleState = !button->getToggleState();
        button->setToggleState(buttonToggleState, NotificationType::dontSendNotification);
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
        bool buttonToggleState = !button->getToggleState();
        button->setToggleState(buttonToggleState, NotificationType::dontSendNotification);
		m_spectrogram2DButton.setToggleState(false, NotificationType::dontSendNotification);

		m_spectrogram2D->setVisible(false);
		m_spectrogram3D->setVisible(buttonToggleState);

		m_spectrogram2D->stop();
		m_spectrogram3D->stop();

		m_activeVisualizer = m_spectrogram3D.get();
		m_activeVisualizer->start();
        resized();
    }
}