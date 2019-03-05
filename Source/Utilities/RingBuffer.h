//--------------------------------------------------------------------------------------------
// Name: RingBuffer.h
// Author: Jérémi Panneton
// Creation date: January 31th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "JuceHeader.h"

/** A circular, lock-free buffer for multiple channels of audio.

    Supports a single writer (producer) and any number of readers (consumers).

    Make sure that the number of samples read from the RingBuffer in every
    readSamples() call is less than the bufferSize specified in the constructor.

    Also, ensure that the number of samples read from the RingBuffer at any time
    plus the number of samples written to the RingBuffer at any time never exceed
    the buffer size. This prevents read/write overlap.
*/
template<class Type>
class RingBuffer
{
public:
    /** Initializes the RingBuffer with the specified channels and size.

        @param numChannels  number of channels of audio to store in buffer
        @param bufferSize   size of the audio buffer
     */
    RingBuffer(int numChannels, int bufferSize)
        : m_audioBuffer(numChannels, bufferSize)
        , m_abstractFifo(bufferSize + 1)
    {
    }

    bool writeSamples(AudioBuffer<Type>& buffer)
    {
        if (m_abstractFifo.getFreeSpace() < buffer.getNumSamples())
            return false;

        int start1, size1, start2, size2;
		m_abstractFifo.prepareToWrite(buffer.getNumSamples(), start1, size1, start2, size2);
        
        for (int i = 0; i < buffer.getNumChannels(); ++i)
        {
            if (size1 > 0)
            {
				m_audioBuffer.copyFrom(i, start1, buffer.getReadPointer(i), size1);
            }

            if (size2 > 0)
            {
				m_audioBuffer.copyFrom(i, start2, buffer.getReadPointer(i) + size1, size2);
            }
        }

		m_abstractFifo.finishedWrite(size1 + size2);
        return true;
    }

    bool readSamples(AudioBuffer<Type>& buffer, double overlapRatio = 0.0)
    {
        if (m_abstractFifo.getNumReady() < buffer.getNumSamples())
            return false;

        int start1, size1, start2, size2;
		m_abstractFifo.prepareToRead(buffer.getNumSamples(), start1, size1, start2, size2);

        for (int i = 0; i < buffer.getNumChannels(); ++i)
        {
            if (size1 > 0)
            {
                buffer.copyFrom(i, 0, m_audioBuffer.getReadPointer(i, start1), size1);
            }

            if (size2 > 0)
            {
                buffer.copyFrom(i, size1, m_audioBuffer.getReadPointer(i, start2), size2);
            }
        }
        
		m_abstractFifo.finishedRead(static_cast<int>((size1 + size2) * (1.0 - overlapRatio)));
        return true;
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
    AbstractFifo m_abstractFifo;
    AudioBuffer<Type> m_audioBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBuffer)
};
