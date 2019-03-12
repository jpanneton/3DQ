//--------------------------------------------------------------------------------------------
// Name: FrequencyAxis.h
// Author: Jérémi Panneton
// Creation date: February 26th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "NormalizedRange.h"
#include <vector>

//--------------------------------------------------------------------------------------------
/// Wraps a discrete bounded frequency axis with user-defined distribution.
//--------------------------------------------------------------------------------------------
template<typename ValueType>
class FrequencyAxis
{
    using FrequencyMap = std::vector<ValueType>;
    using FrequencyRange = NormalizedRange<ValueType>;

public:
    enum class Distribution
    {
        Linear,
        Logarithmic
    };

    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] resolution				Initial number of frequencies the axis should contain.
    /// @param[in] sampleRate				Sample rate.
    //----------------------------------------------------------------------------------------
    template<typename SizeType>
    FrequencyAxis(SizeType resolution, ValueType min, ValueType max)
        : m_frequencyMap(resolution)
        , m_frequencyRange(min, max)
        , m_distribution(Distribution::Logarithmic)
    {
        remapFrequencies();
    }

    //----------------------------------------------------------------------------------------
    /// Sets how the frequencies should be distributed over the axis.
    /// @param[in] type						Distribution type.
    //----------------------------------------------------------------------------------------
    void setDistribution(Distribution type)
    {
        if (type != m_distribution)
        {
            m_distribution = type;
            remapFrequencies();
        }
    }

    //----------------------------------------------------------------------------------------
    /// Sets the resolution (or size) of the axis.
    /// @param[in] resolution				Number of frequencies the axis should contain.
    //----------------------------------------------------------------------------------------
    template<typename SizeType>
    void setResolution(SizeType resolution)
    {
        if (resolution != m_frequencyMap.size())
        {
            m_frequencyMap.resize(resolution);
            remapFrequencies();
        }
    }

    //----------------------------------------------------------------------------------------
    /// Returns the current resolution (or size) of the axis.
    /// @return								Number of frequencies contained in the axis.
    //----------------------------------------------------------------------------------------
    template<typename SizeType = int>
    SizeType getResolution() const noexcept
    {
        return static_cast<SizeType>(m_frequencyMap.size());
    }

    //----------------------------------------------------------------------------------------
    /// Returns the mapped frequency at the specified index.
    /// @param[in] index					Index of the frequency (discrete position on the axis).
    /// @return								Frequency at the specified index.
    //----------------------------------------------------------------------------------------
    ValueType operator[](size_t index) const
    {
        return m_frequencyMap[index];
    }

private:
    //----------------------------------------------------------------------------------------
    /// Maps the frequencies defined in the range according to the current parameters.
    //----------------------------------------------------------------------------------------
    void remapFrequencies()
    {
        const size_t frequencyCount = m_frequencyMap.size();
        jassert(frequencyCount > 0);

        switch (m_distribution)
        {
        case Distribution::Linear:
        {
            for (size_t i = 0; i < frequencyCount; ++i)
            {
                m_frequencyMap[i] = i / ValueType(frequencyCount - 1);
            }

            break;
        }
        case Distribution::Logarithmic:
        {
            for (size_t i = 0; i < frequencyCount; ++i)
            {
                m_frequencyMap[i] = m_frequencyRange.getLogValue(i / ValueType(frequencyCount - 1));
            }

            break;
        }
        }
    }

    FrequencyMap m_frequencyMap;		/// Frequency mapping.
    FrequencyRange m_frequencyRange;	/// Frequency range.
    Distribution m_distribution;		/// Frequency distribution.
};