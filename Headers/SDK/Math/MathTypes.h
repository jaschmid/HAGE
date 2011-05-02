/********************************************************/
/* FILE: MathTypes.h                                    */
/* DESCRIPTION: Basic Math related definitions          */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#include "../types.h"

#ifndef HAGE_MATH_TYPES_H_INCLUDED
#define HAGE_MATH_TYPES_H_INCLUDED


#include <limits>
#include <cmath>

namespace HAGE {

template<typename _T> const _T Zero()
{
	return (_T)0.0;
}
template<typename _T> const _T One()
{
	return (_T)1.0;
}
template<typename _T> const _T NaN()
{
	return (_T)std::numeric_limits<_T>::quiet_NaN();
}

template<typename _T> bool IsZero(const _T& v)
{
	return v == Zero<_T>();
}
template<typename _T> bool IsOne(const _T& v)
{
	return v == One<_T>();
}
template<typename _T> bool IsNaN(const _T& v)
{
	return v != v;
}

template<typename _T> _T sqrt(const _T& v)
{
	return std::sqrt(v);
}

template<> f32 sqrt<f32>(const f32& v)
{
	return std::sqrtf(v);
}

template<> f64 sqrt<f64>(const f64& v)
{
	return std::sqrtl(v);
}

template<class _T = f32> struct Vector2;
template<class _T = f32> struct Vector3;
template<class _T = f32> struct Vector4;
template<class _T = f32> struct Matrix4;
template<class _T = f32> struct Quaternion;

}

#endif
