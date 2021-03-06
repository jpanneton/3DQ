//--------------------------------------------------------------------------------------------
// Name: Math.h
// Author: J�r�mi Panneton
// Creation date: January 24th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include <type_traits>

template<typename T>
constexpr T pi = static_cast<T>(3.14159265358979323846);

// Floors to next integer down to zero infinity
template<typename T>
inline typename std::enable_if_t<std::is_floating_point<T>::value, T>
floorToNInf(T input)
{
    return std::floor(input);
}

template<typename T>
inline typename std::enable_if_t<!std::is_floating_point<T>::value, T>
floorToNInf(T input)
{
    return input;
}