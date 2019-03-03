//--------------------------------------------------------------------------------------------
// Name: Spectrogram.h
// Author: J�r�mi Panneton
// Creation date: February 5th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "OpenGLComponent.h"
#include "Utilities/FrequencyAxis.h"

//--------------------------------------------------------------------------------------------
/// Base spectrogram interface. Contains all the data and processing needed for visualization.
/// Visualization and rendering must be performed in derived class.
//--------------------------------------------------------------------------------------------
class Spectrogram : public OpenGLComponent
{
public:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] ringBuffer               Reference to the audio thread's ring buffer.
    /// @param[in] sampleRate               Sample rate.
    /// @param[in] outputResolution         Frequency output resolution.
    //----------------------------------------------------------------------------------------
    Spectrogram(RingBuffer<GLfloat>& ringBuffer, double sampleRate, int outputResolution);

    //----------------------------------------------------------------------------------------
    /// Destructor.
    //----------------------------------------------------------------------------------------
    ~Spectrogram();

protected:
	struct FrequencyInfo
	{
		float frequency = {};
		float level = {};
	};

    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::initialise.
    //----------------------------------------------------------------------------------------
    virtual void initialise() = 0;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::createShaders.
    //----------------------------------------------------------------------------------------
    virtual void createShaders() = 0;

	//----------------------------------------------------------------------------------------
	/// Updates the data by performing an FFT on the current audio frame.
	/// The FFT output is then getting averaged and interpolated for a smoother result.
	/// This method should be called before each render.
	/// @return								True if the data has been updated successfully.
	//----------------------------------------------------------------------------------------
    bool updateData();

	//----------------------------------------------------------------------------------------
	/// Returns the frequency and the according level (in normalized dB scale) at the specified index in the frequency axis.
	/// @param[in] index					Position of the frequency on the axis.
	/// @return								Frequency and level (in normalized dB scale).
	//----------------------------------------------------------------------------------------
	FrequencyInfo getFrequencyInfo(int index) const;

    //----------------------------------------------------------------------------------------
    /// @see OpenGLComponent::render.
    //----------------------------------------------------------------------------------------
    virtual void render() = 0;

    //----------------------------------------------------------------------------------------
    /// @see Component::resized.
    //----------------------------------------------------------------------------------------
    void resized() override;

    //----------------------------------------------------------------------------------------
    /// @see MouseListener::mouseEnter.
    //----------------------------------------------------------------------------------------
    void mouseEnter(const MouseEvent& event) override;

    //----------------------------------------------------------------------------------------
    /// @see MouseListener::mouseMove.
    //----------------------------------------------------------------------------------------
    void mouseMove(const MouseEvent& event) override;

    //----------------------------------------------------------------------------------------
    /// @see MouseListener::mouseMove.
    //----------------------------------------------------------------------------------------
    void mouseExit(const MouseEvent& event) override;

    enum
    {
        fftOrder = 12,
        fftSize = 1 << fftOrder, // 2 ^ fftOrder
        fftBins = fftSize >> 1 // fftSize / 2
    };

    // Overlay GUI
    Label m_statusLabel;					/// GUI label used to display various informations about the current state of the spectrogram.
    FrequencyAxis<float> m_frequencyAxis;	/// Frequency axis used for frequency data scaling.
	ColourGradient m_colorMap;				/// Color gradient used as colormap for the normalized levels.

	std::atomic_bool m_isMouseHover = {};	/// If true, the mouse is inside the display frame. If false, the mouse is out of bounds.
    Point<int> m_mousePosition;				/// Current mouse position in local coordinates.

private:
    enum class InterpolationMode
    {
        None,
        Linear,
        Lanczos
    };

    void interpolateData(const float* inputData, float* outputData, InterpolationMode interpolationMode);

    // Audio structures
    dsp::FFT m_forwardFFT;					/// Forward Fourier transform function.
    dsp::WindowingFunction<float> m_window;	/// Window function used to smooth spectral leakage.

	HeapBlock<float, true> m_fftData;		/// Data used for FFT (as input and output).
	HeapBlock<float, true> m_visuData;		/// Final output data used for visualisation.

    AudioBuffer<float> m_averager;			/// Averaged FFT output (used for smoother frequency resolution).
    int m_averagerPtr = 1;					/// Index used to keep track of the oldest averager slot.

	const bool m_adaptativeLevel = true;	/// If true, the level is normalized using min et max levels. If false, the original level is used for visualization.
	float m_maxFFTLevel = {};				/// Maximum level of the latest FFT frame.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrogram)
};