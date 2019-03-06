//--------------------------------------------------------------------------------------------
// Name: RingBuffer.h
// Author: Jérémi Panneton
// Creation date: January 31th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "AbstractRingBuffer.h"

/** A circular, lock-free buffer for multiple channels of audio.

    Supports a single writer (producer) and any number of readers (consumers).

    Make sure that the number of samples read from the RingBuffer in every
    readSamples() call is less than the bufferSize specified in the constructor.

    Also, ensure that the number of samples read from the RingBuffer at any time
    plus the number of samples written to the RingBuffer at any time never exceed
    the buffer size. This prevents read/write overlap.
*/
template<typename T>
class RingBuffer
{
public:
	using ValueType = T;

    /** Initializes the RingBuffer with the specified channels and size.

        @param numChannels  number of channels of audio to store in buffer
        @param bufferSize   size of the audio buffer
     */
    RingBuffer(int numChannels, int bufferSize)
        : m_audioBuffer(numChannels, bufferSize)
        , m_abstractFifo(bufferSize + 1)
    {
    }

    bool writeSamples(AudioBuffer<ValueType>& buffer)
    {
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

    bool readSamples(AudioBuffer<ValueType>& buffer, double overlapRatio = 0.0)
    {
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

    void setVirtualSize(int readSize, int chunkCount = 10)
    {
        const int bufferSize = readSize * chunkCount;
        if (bufferSize != m_abstractFifo.getTotalSize())
        {
            jassert(bufferSize <= m_audioBuffer.getNumSamples());
			m_abstractFifo.setTotalSize(bufferSize + 1);
        }
    }

private:
	AbstractRingBuffer m_abstractFifo;
    AudioBuffer<ValueType> m_audioBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBuffer)
};