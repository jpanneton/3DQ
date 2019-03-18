//--------------------------------------------------------------------------------------------
// Name: Filters.h
// Author: Jérémi Panneton
// Creation date: January 30th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include "Utilities/Math.h"

// https://bitbucket.org/Mayae/cpl

template<typename T>
inline T lanczosKernel(T x, int size)
{
    return x ? (size * sin(pi<T> * x) * sin(pi<T> * x / size)) / (pi<T> * pi<T> * x * x) : T(1);
}

template<typename T>
inline T lanczosFilter(const T* vec, int asize, T x, int wsize)
{
    T resonance = 0;
    int start = static_cast<int>(floorToNInf(x));
    for (int i = start - wsize + 1; i < (start + wsize + 1); ++i)
    {
        if (i >= 0 && i < asize)
        {
            auto impulse = vec[i];
            auto response = lanczosKernel(x - i, wsize);
            resonance += impulse * response;
        }
    }
    return resonance;
}

template<typename T>
inline T linearFilter(const T* vec, int asize, T x)
{
    int x1 = floorToNInf<int>(static_cast<int>(x));
    int x2 = std::min(asize - 1, x1 + 1);

    auto frac = x - x1;
    return vec[x1] * (1 - frac) + vec[x2] * frac;
}