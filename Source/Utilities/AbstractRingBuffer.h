//--------------------------------------------------------------------------------------------
// Name: AbstractRingBuffer.h
// Author: Jérémi Panneton
// Creation date: March 5th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#pragma warning(push)
#pragma warning(disable : 4324) // C4324: padding due to alignment

#include "JuceHeader.h"
#include <atomic>
#include <new>

//--------------------------------------------------------------------------------------------
/// Encapsulates the logic required to implement a single-reader single-writer lock-free FIFO (doesn't hold any data).
//--------------------------------------------------------------------------------------------
class AbstractRingBuffer
{
public:
    //----------------------------------------------------------------------------------------
    /// Constructor.
    /// @param[in] capacity					Total size of the buffer being managed.
    //----------------------------------------------------------------------------------------
    AbstractRingBuffer(int capacity) noexcept;

    //----------------------------------------------------------------------------------------
    /// Updates the total size of the buffer being managed.
    /// @warning							Not thread-safe! Shouldn't be called while reading or writing.
    /// @param[in] capacity					Total size of the buffer being managed.
    //----------------------------------------------------------------------------------------
    void setTotalSize(int capacity) noexcept;

    //----------------------------------------------------------------------------------------
    /// Returns the total size of the buffer being managed.
    //----------------------------------------------------------------------------------------
    int getTotalSize() const noexcept;

    //----------------------------------------------------------------------------------------
    /// Returns the number of items that can be added to the buffer without overflowing.
    //----------------------------------------------------------------------------------------
    int getFreeSpace() const noexcept;

    //----------------------------------------------------------------------------------------
    /// Returns the number of items in the buffer.
    //----------------------------------------------------------------------------------------
    int getNumReady() const noexcept;

    //----------------------------------------------------------------------------------------
    /// Clears the buffer positions, so that it appears empty.
    //----------------------------------------------------------------------------------------
    void reset() noexcept;

private:
    //----------------------------------------------------------------------------------------
    /// Wraps the information needed to perform the actual read or write operation.
    /// Since it's a ring buffer, the requested block can span from the end of the buffer to the beginning.
    /// So, the requested block is handled as two separate blocks.
    //----------------------------------------------------------------------------------------
    struct OperationResult
    {
        int startIndex1;	/// Index of the first part of the requested block.
        int blockSize1;		/// Size of the first part of the requested block.
        int startIndex2;	/// Index of the second part of the requested block (always 0).
        int blockSize2;		/// Size of the second part of the requested block (0 if no second part).
    };

    //----------------------------------------------------------------------------------------
    /// Returns the current read or write location of a block of data within the buffer.
    /// When reading, the targeted buffer area should be the one filled with items.
    /// When writing, the targeted buffer area should be the free space.
    /// @param[in] firstIndex               First index of the targeted buffer area (begin). It should be the head when reading and the tail when writing.
    /// @param[in] lastIndex				Last index of the targeted buffer area (end). It should be the tail when reading and the head when writing.
    /// @param[in] blockSize				Size of the block to read or write.
    /// @return								Information needed to perform the actual read or write operation.
    //----------------------------------------------------------------------------------------
    OperationResult generateResult(int firstIndex, int lastIndex, int blockSize) const noexcept;

public:
    //----------------------------------------------------------------------------------------
    /// Performs a full read operation using this abstract buffer.
    /// @param[in] numToRead	            Number of items to read from the buffer (guaranteed).
    /// @param[in] readOperation	        Actual read operation. The lambda should take an OperationResult as parameter (or auto) and return the final number of items read from the buffer.
    /// @return								False if the requested number of items to read from the buffer is too large. True otherwise.
    //----------------------------------------------------------------------------------------
    template<class Lambda>
    bool read(int numToRead, Lambda readOperation)
    {
        // Relaxed, because the writer thread will never change m_head
        const auto head = m_head.load(std::memory_order_relaxed);
        // Acquire, because the writer thread is the one that changes m_tail
        const auto tail = m_tail.load(std::memory_order_acquire);

        // Equivalent of calling getNumReady(), but faster since the atomic accesses
        // are already done and more optimized
        const auto numReady = (tail >= head ? tail - head : getTotalSize() - (head - tail));
        if (numToRead > numReady)
            return false;

        // Perform the actual read operation
        const int numRead = readOperation(generateResult(head, tail, numToRead));

        // Update the state of the virtual FIFO
        jassert(numRead >= 0 && numRead <= getTotalSize());
        auto newHead = head + numRead;

        if (newHead >= getTotalSize())
        {
            newHead -= getTotalSize();
        }

        // Release, because the writer thread may try to acquire m_head
        m_head.store(newHead, std::memory_order_release);
        return true;
    }

    //----------------------------------------------------------------------------------------
    /// Performs a full write operation using this abstract buffer.
    /// @param[in] numToWrite	            Number of items to write in the buffer (guaranteed).
    /// @param[in] writeOperation	        Actual write operation. The lambda should take an OperationResult as parameter (or auto) and return the final number of items written in the buffer.
    /// @return								False if the requested number of items to write in the buffer is too large. True otherwise.
    //----------------------------------------------------------------------------------------
    template<class Lambda>
    bool write(int numToWrite, Lambda writeOperation)
    {
        // Relaxed, because the reader thread will never change m_tail
        const auto tail = m_tail.load(std::memory_order_relaxed);
        // Acquire, because the reader thread is the one that changes m_head
        const auto head = m_head.load(std::memory_order_acquire);

        // Equivalent of calling getFreeSpace(), but faster since the atomic accesses
        // are already done and more optimized
        const auto freeSpace = (tail >= head ? getTotalSize() - (tail - head) : head - tail) - 1;
        if (numToWrite > freeSpace)
            return false;

        // Perform the actual write operation
        const int numWritten = writeOperation(generateResult(tail, head, numToWrite));

        // Update the state of the virtual FIFO
        jassert(numWritten >= 0 && numWritten < getTotalSize());
        auto newTail = tail + numWritten;

        if (newTail >= getTotalSize())
        {
            newTail -= getTotalSize();
        }

        // Release, because the reader thread may try to acquire m_tail
        m_tail.store(newTail, std::memory_order_release);
        return true;
    }

private:
    // Cache line size used to align head and tail properly and avoid false sharing
    static constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;

    int m_bufferSize = 0;	/// Total size of the buffer being managed.
    
    // Align to avoid false sharing between head and tail
    alignas(CACHE_LINE_SIZE) std::atomic<int> m_head = 0;	/// Head / front of the virtual FIFO (used for read operations).
    alignas(CACHE_LINE_SIZE) std::atomic<int> m_tail = 0;	/// Tail / back of the virtual FIFO (used for write operations).

    // Padding to avoid adjacent allocations to the same cache line as tail
    char m_padding[CACHE_LINE_SIZE - sizeof(decltype(m_tail))];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbstractRingBuffer)
};

#pragma warning(pop)