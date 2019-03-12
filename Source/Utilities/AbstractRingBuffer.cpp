//--------------------------------------------------------------------------------------------
// Name: AbstractRingBuffer.cpp
// Author: Jérémi Panneton
// Creation date: March 5th, 2019
//--------------------------------------------------------------------------------------------

#include "AbstractRingBuffer.h"

AbstractRingBuffer::AbstractRingBuffer(int capacity) noexcept
    : m_bufferSize(capacity)
{
    jassert(m_bufferSize > 0);
}

void AbstractRingBuffer::setTotalSize(int capacity) noexcept
{
    jassert(capacity > 0);
    reset();
    m_bufferSize = capacity;
}

int AbstractRingBuffer::getTotalSize() const noexcept
{
    return m_bufferSize;
}

int AbstractRingBuffer::getFreeSpace() const noexcept
{
    return getTotalSize() - getNumReady() - 1;
}

int AbstractRingBuffer::getNumReady() const noexcept
{
    const auto head = m_head.load();
    const auto tail = m_tail.load();
    return tail >= head ? (tail - head) : (getTotalSize() - (head - tail));
}

void AbstractRingBuffer::reset() noexcept
{
    m_tail = 0;
    m_head = 0;
}

AbstractRingBuffer::OperationResult AbstractRingBuffer::generateResult(int firstIndex, int lastIndex, int blockSize) const noexcept
{
    OperationResult result;

    if (blockSize <= 0)
    {
        result.startIndex1 = 0;
        result.startIndex2 = 0;
        result.blockSize1 = 0;
        result.blockSize2 = 0;
    }
    else
    {
        result.startIndex1 = firstIndex;
        result.startIndex2 = 0;
        result.blockSize1 = jmin(getTotalSize() - firstIndex, blockSize);
        blockSize -= result.blockSize1;
        result.blockSize2 = blockSize <= 0 ? 0 : jmin(blockSize, lastIndex);
    }

    return result;
}