//--------------------------------------------------------------------------------------------
// Name: Spectrogram.cpp
// Author: J�r�mi Panneton
// Creation date: February 5th, 2019
//--------------------------------------------------------------------------------------------

#include "Spectrogram.h"
#include "DSP/Filters.h"
#include "Utilities/ColorGradients.h"
#include <numeric>

Spectrogram::Spectrogram(double sampleRate, int outputResolution, StatusBar& statusBar)
    : OpenGLComponent(fftSize, sampleRate, false)
    , m_statusBar(statusBar)
    , m_frequencyAxis(outputResolution, 20.0f, static_cast<float>(sampleRate) / 2) // Nyquist frequency
    , m_colorMap(64)
    , m_forwardFFT(fftOrder)
    , m_window(fftSize, dsp::WindowingFunction<float>::hann)
    , m_fftData(2 * fftSize, true)
    , m_visuData(outputResolution, true)
    , m_averager(5, fftBins)
{
    m_averager.clear();

    // Default colormap
    m_colorMap.setGradient(ColorGradients::getDefaultGradient());
}

Spectrogram::~Spectrogram()
{
    // Turn off OpenGL
    shutdownOpenGL();
}

void Spectrogram::setMaxFrequency(float frequency, const ColourGradient& gradient)
{
    m_colorMap.setGradient(gradient);
    m_frequencyAxis.setMaxFrequency(frequency);
}

void Spectrogram::setAdaptiveLevel(bool enabled)
{
    m_adaptativeLevel = enabled;
}

void Spectrogram::setClipLevel(bool enabled)
{
    m_clipLevel = enabled;
}

//==========================================================================
// OpenGL Callbacks
bool Spectrogram::updateData()
{
    // Copy data from ring buffer into FFT
    m_ringBuffer.readSamples(m_readBuffer, 0.5);
    
    // Zero Out FFT for next use
    zeromem(m_fftData, sizeof(GLfloat) * 2 * fftSize);
    FloatVectorOperations::clear(m_fftData, getReadSize());

    /** Future Feature:
        Instead of summing channels below, keep the channels separate and
        lay out the spectrum so you can see the left and right channels
        individually on either half of the spectrum.
     */
     // Sum channels together
     // Should be average
    for (int i = 0; i < 2; ++i)
    {
        FloatVectorOperations::add(m_fftData, m_readBuffer.getReadPointer(i), getReadSize());
    }

    // Apply window to avoid any spectral leakage
    m_window.multiplyWithWindowingTable(m_fftData, fftSize);
    // Perform FFT
    m_forwardFFT.performFrequencyOnlyForwardTransform(m_fftData);
    // Average FFT output to smooth frequency resolution (Welch's method)
    m_averager.addFrom(0, 0, m_averager.getReadPointer(m_averagerPtr), m_averager.getNumSamples(), -1.0f);
    m_averager.copyFrom(m_averagerPtr, 0, m_fftData, m_averager.getNumSamples(), 1.0f / (m_averager.getNumSamples() * (m_averager.getNumChannels() - 1)));
    m_averager.addFrom(0, 0, m_averager.getReadPointer(m_averagerPtr), m_averager.getNumSamples());
    if (++m_averagerPtr == m_averager.getNumChannels())
        m_averagerPtr = 1;
    
    const float* averagedData = m_averager.getReadPointer(0);

    // Find the range of values produced, so we can scale our rendering to show up the detail clearly
    m_fftLevelRange = FloatVectorOperations::findMinAndMax(averagedData, fftBins);

    // Interpolate the latest averaged result
    interpolateData(averagedData, m_visuData, InterpolationMode::Lanczos);

    return true;
}

Spectrogram::FrequencyInfo Spectrogram::getFrequencyInfo(int index) const
{
    const float frequency = m_frequencyAxis[index];
    const float sample = m_visuData[index];
    float leveldB = 0.0f;
    float level = 0.0f;

    if (m_fftLevelRange.getEnd() != 0.0f)
    {
        const float mindB = m_adaptativeLevel ? Decibels::gainToDecibels(m_fftLevelRange.getStart()) : -90.0f; // -100
        const float maxdB = m_adaptativeLevel ? Decibels::gainToDecibels(m_fftLevelRange.getEnd()) : 10.0f;
        if (mindB < maxdB)
        {
            leveldB = Decibels::gainToDecibels(sample);
            if (!m_adaptativeLevel && m_clipLevel && level > 0.0f)
            {
                leveldB = 0.0f;
            }
            level = jmap(jlimit(mindB, maxdB, leveldB), mindB, maxdB, 0.0f, 1.0f);
        }
    }

    return { frequency, leveldB, level };
}

void Spectrogram::mouseEnter(const MouseEvent&)
{
    m_isMouseHover = true;
}

void Spectrogram::mouseMove(const MouseEvent& event)
{
    m_mousePosition = event.getPosition();
    m_mousePosition.y = getHeight() - m_mousePosition.y - 1;
}

void Spectrogram::mouseExit(const MouseEvent&)
{
    m_isMouseHover = false;
}

void Spectrogram::interpolateData(const float* inputData, float* outputData, InterpolationMode interpolationMode)
{
    FloatVectorOperations::clear(outputData, m_frequencyAxis.getResolution());

    const int lanczosFilterSize = 5;
    const float nyquistFrequency = static_cast<float>(m_sampleRate) / 2;
    // Use frequency axis range instead of Nyquist frequency
    const float freqToBin = (fftBins - 1) / nyquistFrequency;

    /*for (size_t i = 0; i < fftBins; ++i)
    {
        inputData[i] = std::abs(inputData[i]);
    }*/

    double fftBinWidth = 1.0 / fftBins;
    size_t x = 0;

    // 1- Interpolate lower frequencies
    switch (interpolationMode)
    {
    case InterpolationMode::None:
        for (x = 0; x < m_frequencyAxis.getResolution() - 1; ++x)
        {
            const double freqBinWidth = (m_frequencyAxis[x + 1] - m_frequencyAxis[x]) / nyquistFrequency;
            if (freqBinWidth > fftBinWidth)
                break;
            // + 0.5 to centerly space bins
            size_t index = jlimit(size_t(0), size_t(fftBins - 1), size_t(m_frequencyAxis[x] * freqToBin + 0.5));
            outputData[x] = inputData[index];
        }
        break;
    case InterpolationMode::Linear:
        for (x = 0; x < m_frequencyAxis.getResolution() - 1; ++x)
        {
            const double freqBinWidth = (m_frequencyAxis[x + 1] - m_frequencyAxis[x]) / nyquistFrequency;
            if (freqBinWidth > fftBinWidth)
                break;
            outputData[x] = linearFilter(inputData, fftBins, m_frequencyAxis[x] * freqToBin);
        }
        break;
    case InterpolationMode::Lanczos:
        for (x = 0; x < m_frequencyAxis.getResolution() - 1; ++x)
        {
            const double freqBinWidth = (m_frequencyAxis[x + 1] - m_frequencyAxis[x]) / nyquistFrequency;
            if (freqBinWidth > fftBinWidth)
                break;
            outputData[x] = lanczosFilter(inputData, fftBins, m_frequencyAxis[x] * freqToBin, lanczosFilterSize);
        }
        break;
    }

    // 2- Filter out higher frequencies
    int lastBin = static_cast<int>(m_frequencyAxis[x] * freqToBin);
    for (; x < m_frequencyAxis.getResolution(); ++x)
    {
        const int currentBin = static_cast<int>(m_frequencyAxis[x] * freqToBin);

        int maxBin = currentBin;
        float maxBinLevel = std::numeric_limits<float>::lowest();

        // Loop over all the bins that is mapped for a single coordinate
        for (int nextBin = lastBin + 1; nextBin <= currentBin; ++nextBin)
        {
            const float nextBinLevel = inputData[nextBin];
            // Select the one with the highest level for display
            if (nextBinLevel > maxBinLevel)
            {
                maxBin = nextBin;
                maxBinLevel = nextBinLevel;
            }
        }

        outputData[x] = inputData[maxBin];
        lastBin = currentBin;
    }
}