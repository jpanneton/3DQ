//--------------------------------------------------------------------------------------------
// Name: RingBuffer.h
// Author: J�r�mi Panneton
// Creation date: January 31th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "AbstractRingBuffer.h"

//--------------------------------------------------------------------------------------------
/// Single-reader single-writer lock-free FIFO.
//--------------------------------------------------------------------------------------------
template<typename T>
class RingBuffer
{
public:
    using ValueType = T;

    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] channelCount				Number of buffers (one for each channel).
    /// @param[in] bufferSize				Total size of one buffer.
    //----------------------------------------------------------------------------------------
    RingBuffer(int channelCount, int bufferSize)
        : m_audioBuffer(channelCount, bufferSize)
        , m_abstractFifo(bufferSize)
    {
    }

    //----------------------------------------------------------------------------------------
    /// Adds audio data to the queue.
    /// @param[in] buffer					Buffer containing audio data.
    /// @return								False if the requested number of items to write in the buffer is too large. True otherwise.
    //----------------------------------------------------------------------------------------
    bool writeSamples(const AudioBuffer<ValueType>& buffer)
    {
        jassert(buffer.getNumChannels() == m_audioBuffer.getNumChannels());
        return m_abstractFifo.write(buffer.getNumSamples(), [&](const auto& result)
        {
            for (int i = 0; i < buffer.getNumChannels(); ++i)
            {
                if (result.blockSize1 > 0)
                {
                    m_audioBuffer.copyFrom(i, result.startIndex1, buffer.getReadPointer(i), result.blockSize1);
                }

                if (result.blockSize2 > 0)
                {
                    m_audioBuffer.copyFrom(i, result.startIndex2, buffer.getReadPointer(i) + result.blockSize1, result.blockSize2);
                }
            }
            return result.blockSize1 + result.blockSize2;
        });
    }

    //----------------------------------------------------------------------------------------
    /// Pops audio data from the queue. Since it's a FIFO, the oldest data is retrieved.
    /// @param[out] buffer					Buffer in which to store the audio data.
    /// @param[in] overlapRatio				Ratio of data to keep in the queue after being read. If 0.0 (min value), all the requested data is read and removed from the queue. If 1.0 (max value), all the requested data is read and none is removed from the queue.
    /// @return								False if the requested number of items to read from the buffer is too large. True otherwise.
    //----------------------------------------------------------------------------------------
    bool readSamples(AudioBuffer<ValueType>& buffer, double overlapRatio = 0.0)
    {
        jassert(buffer.getNumChannels() == m_audioBuffer.getNumChannels());
        return m_abstractFifo.read(buffer.getNumSamples(), [&](const auto& result)
        {
            for (int i = 0; i < buffer.getNumChannels(); ++i)
            {
                if (result.blockSize1 > 0)
                {
                    buffer.copyFrom(i, 0, m_audioBuffer.getReadPointer(i, result.startIndex1), result.blockSize1);
                }

                if (result.blockSize2 > 0)
                {
                    buffer.copyFrom(i, result.blockSize1, m_audioBuffer.getReadPointer(i, result.startIndex2), result.blockSize2);
                }
            }
            return static_cast<int>((result.blockSize1 + result.blockSize2) * (1.0 - overlapRatio));
        });
    }

    //----------------------------------------------------------------------------------------
    /// Sets the virtual size of the queue. No reallocation is being done, but the logical size of the queue is changed.
    /// The new size is calculated by doing readSize * chunkCount. This size should be less than the real size of the buffer.
    /// @warning							Not thread-safe! Shouldn't be called while reading or writing.
    /// @param[in] readSize					Expected read size.
    /// @param[in] chunkCount				Number of reads needed to traverse the whole ring buffer.
    //----------------------------------------------------------------------------------------
    void setVirtualSize(int readSize, int chunkCount = 10) noexcept
    {
        const int bufferSize = readSize * chunkCount;
        if (bufferSize != m_abstractFifo.getTotalSize())
        {
            jassert(bufferSize <= m_audioBuffer.getNumSamples());
            m_abstractFifo.setTotalSize(bufferSize);
        }
    }

    //----------------------------------------------------------------------------------------
    /// Clears the queue. No reallocation is being done, but the logical size of the queue is resetted.
    //----------------------------------------------------------------------------------------
    void clear() noexcept
    {
        m_abstractFifo.reset();
    }

private:
    AbstractRingBuffer m_abstractFifo;
    AudioBuffer<ValueType> m_audioBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBuffer)
};