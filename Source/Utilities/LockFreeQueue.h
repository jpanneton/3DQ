/*
Copyright (c) 2018 Erik Rigtorp <erik@rigtorp.se>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// https://github.com/rigtorp/SPSCQueue/blob/master/include/rigtorp/SPSCQueue.h

#pragma once

#include <atomic>
#include <cassert>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>

template<class T>
class LockFreeQueue
{
#define NO_EXCEPT(...) noexcept(std::is_nothrow_constructible_v<__VA_ARGS__>)

    using ssize_t = std::ptrdiff_t; // ou std::intmax_t

public:
    explicit LockFreeQueue(std::size_t capacity)
        : m_capacity(capacity)
        , m_slots(capacity < 2 ? throw std::invalid_argument("size < 2")
            // call to global new[] operator (raw allocation)
            : static_cast<T*>(operator new[]((capacity + 2 * PADDING) * sizeof(T))))
        , m_head(0)
        , m_tail(0)
    {
        static_assert(alignof(LockFreeQueue<T>) >= CACHE_LINE_SIZE);
        assert(reinterpret_cast<std::byte*>(&m_tail) -
            reinterpret_cast<char*>(&m_head) >=
            static_cast<ssize_t>(CACHE_LINE_SIZE));
    }

    ~LockFreeQueue()
    {
        while (front()) {
            pop();
        }
        operator delete[](m_slots);
    }

    // non-copyable and non-movable
    LockFreeQueue(const LockFreeQueue &) = delete;
    LockFreeQueue &operator=(const LockFreeQueue &) = delete;

    template<class... Args>
    void emplace(Args&&... args) NO_EXCEPT(T, Args&&...)
    {
        static_assert(std::is_constructible<T, Args &&...>::value,
            "T must be constructible with Args&&...");
        auto const head = m_head.load(std::memory_order_relaxed);
        auto nextHead = head + 1;
        if (nextHead == m_capacity)
        {
            nextHead = 0;
        }
        while (nextHead == m_tail.load(std::memory_order_acquire))
            ;
        new (&m_slots[head + PADDING]) T(std::forward<Args>(args)...);
        m_head.store(nextHead, std::memory_order_release);
    }

    template<class... Args>
    bool try_emplace(Args&&... args) NO_EXCEPT(T, Args&&...)
    {
        static_assert(std::is_constructible<T, Args &&...>::value,
            "T must be constructible with Args&&...");
        auto const head = m_head.load(std::memory_order_relaxed);
        auto nextHead = head + 1;
        if (nextHead == m_capacity)
        {
            nextHead = 0;
        }
        if (nextHead == m_tail.load(std::memory_order_acquire))
        {
            return false;
        }
        new (&m_slots[head + PADDING]) T(std::forward<Args>(args)...);
        m_head.store(nextHead, std::memory_order_release);
        return true;
    }

    void push(const T& v) NO_EXCEPT(T)
    {
        static_assert(std::is_copy_constructible<T>::value,
            "T must be copy constructible");
        emplace(v);
    }

    template<typename P, typename = typename std::enable_if<std::is_constructible<T, P &&>::value>::type>
        void push(P&& v) NO_EXCEPT(T, P&&)
    {
        emplace(std::forward<P>(v));
    }

    bool try_push(const T &v) NO_EXCEPT(T)
    {
        static_assert(std::is_copy_constructible<T>::value,
            "T must be copy constructible");
        return try_emplace(v);
    }

    template<typename P, typename = typename std::enable_if<
        std::is_constructible<T, P &&>::value>::type>
        bool try_push(P&& v) NO_EXCEPT(T, P&&)
    {
        return try_emplace(std::forward<P>(v));
    }

    T* front() noexcept
    {
        auto const tail = m_tail.load(std::memory_order_relaxed);
        if (m_head.load(std::memory_order_acquire) == tail)
            return nullptr;
        return &m_slots[tail + PADDING];
    }

    void pop() noexcept
    {
        static_assert(std::is_nothrow_destructible<T>::value,
            "T must be nothrow destructible");
        auto const tail = m_tail.load(std::memory_order_relaxed);
        assert(m_head.load(std::memory_order_acquire) != tail);
        m_slots[tail + PADDING].~T();
        auto nextTail = tail + 1;
        if (nextTail == m_capacity) {
            nextTail = 0;
        }
        m_tail.store(nextTail, std::memory_order_release);
    }

    std::size_t size() const noexcept
    {
        ssize_t diff = m_head.load(std::memory_order_acquire) -
            m_tail.load(std::memory_order_acquire);
        if (diff < 0) {
            diff += m_capacity;
        }
        return diff;
    }

    bool empty() const noexcept { return size() == 0; }

    std::size_t capacity() const noexcept { return m_capacity; }

private:
    static constexpr std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;

    // Padding to avoid false sharing between slots and adjacent allocations
    static constexpr std::size_t PADDING = (CACHE_LINE_SIZE - 1) / sizeof(T) + 1;

private:
    const std::size_t m_capacity;
    T *const m_slots;

    // Align to avoid false sharing between head and tail
    alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> m_head;
    alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> m_tail;

    // Padding to avoid adjacent allocations to the same cache line as tail
    char m_padding[CACHE_LINE_SIZE - sizeof(decltype(m_tail))];
};