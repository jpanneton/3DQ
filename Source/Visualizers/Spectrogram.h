//--------------------------------------------------------------------------------------------
// Name: Spectrogram.h
// Author: Jérémi Panneton
// Creation date: February 5th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "GUI/OpenGLComponent.h"
#include "Utilities/ColorMap.h"
#include "Utilities/FrequencyAxis.h"

class StatusBar;

//--------------------------------------------------------------------------------------------
/// Base spectrogram interface. Contains all the data and processing needed for visualization.
/// Visualization and rendering must be performed in a derived class.
//--------------------------------------------------------------------------------------------
class Spectrogram : public OpenGLComponent
{
public:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] sampleRate               Sample rate.
    /// @param[in] outputResolution         Frequency output resolution.
    /// @param[out] statusBar               Reference to the status bar (GUI).
    //----------------------------------------------------------------------------------------
    Spectrogram(double sampleRate, int outputResolution, StatusBar& statusBar);

    //----------------------------------------------------------------------------------------
    /// Destructor.
    //----------------------------------------------------------------------------------------
    virtual ~Spectrogram();

    //----------------------------------------------------------------------------------------
    /// Set the maximum frequency of the spectrogram to better visualize the according range.
    /// @param[in] frequency                Maximum frequency.
    /// @param[in] gradient                 Color gradient to use as a colormap.
    //----------------------------------------------------------------------------------------
    void setMaxFrequency(float frequency, const ColourGradient& gradient);

    //----------------------------------------------------------------------------------------
    /// Set the adaptive level mode on or off.
    /// @param[in] enabled                  If true, the level is normalized using min et max levels. If false, the original level is used for visualization.
    //----------------------------------------------------------------------------------------
    void setAdaptiveLevel(bool enabled);

    //----------------------------------------------------------------------------------------
    /// Set the clip level mode on or off.
    /// @param[in] enabled                  If true, the level is clipped to 0 dB. If false, the level is clipped to an arbitrary positive dB value.
    //----------------------------------------------------------------------------------------
    void setClipLevel(bool enabled);

protected:
    struct FrequencyInfo
    {
        float frequency = {};           /// Frequency.
        float dbLevel = {};             /// Level in dB.
        float normalizedLevel = {};     /// Normalized level (between 0 and 1). Should be used for display.
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

    StatusBar& m_statusBar;                 /// Reference to the status bar (GUI). The component should be updated in a derived class.

    FrequencyAxis<float> m_frequencyAxis;	/// Frequency axis used for frequency data scaling.
    ColorMap m_colorMap;					/// Color map used for the normalized levels.

    std::atomic_bool m_isMouseHover = {};	/// If true, the mouse is inside the display frame. If false, the mouse is out of bounds.
    Point<int> m_mousePosition;				/// Current mouse position in local coordinates (relative to the bottom left corner).

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

    Range<float> m_fftLevelRange;		    /// Minimum and maximum levels of the latest FFT frame.
    bool m_adaptativeLevel = false;	        /// If true, the level is normalized using min et max levels. If false, the original level is used for visualization.
    bool m_clipLevel = false;               /// If true, the level is clipped to 0 dB. If false, the level is clipped to an arbitrary positive dB value.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrogram)
};