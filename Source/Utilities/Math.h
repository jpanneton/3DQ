//--------------------------------------------------------------------------------------------
// Name: Math.h
// Author: Jérémi Panneton
// Creation date: January 24th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

#include <type_traits>

template<typename T>
constexpr T pi = static_cast<T>(3.14159265358979323846);

// floors to next integer, down to zero infinity
template<typename T>
inline typename std::enable_if<std::is_floating_point<T>::value, T>::type
floorToNInf(T input)
{
    return std::floor(input);
}

template<typename T>
inline typename std::enable_if<!std::is_floating_point<T>::value, T>::type
floorToNInf(T input)
{
    return input;
}