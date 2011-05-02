/********************************************************/
/* FILE: Vector2.h                                      */
/* DESCRIPTION: Hage Vector2 class                      */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef HAGE_MATH_VECTOR2_H_INCLUDED
#define HAGE_MATH_VECTOR2_H_INCLUDED

namespace HAGE {


template<typename _T = f32> struct Vector2
{
public:
	union
	{
		_T c[2];
		struct
		{
			_T x,y;
		};
	};

	// access functions

	_T& operator[](const up& i)
	{
		return c[i];
	}

	// const access functions

	const _T& operator[](const up& i) const
	{
		return c[i];
	}

	// constructors

	Vector2(const _T& _1,const _T& _2) : x(_1),y(_2)
	{
	}

	template<class _T2> Vector2(_T2 _1,_T2 _2) : x((const _T&)_1),y((const _T&)_2)
	{
	}

	Vector2(){};

	// math operations
	// scalar

	template<class _T2> Vector2<_T> operator *(const _T2& v) const
	{
		return Vector2<_T>(c[0] * (_T)v,c[1] * (_T)v);
	}

	template<class _T2> Vector2<_T> operator /(const _T2& v) const
	{
		return Vector2<_T>(c[0] / (_T)v,c[1] / (_T)v);
	}

	// vector

	template<class _T2> Vector2<_T> operator +(const Vector2<_T2>& v) const
	{
		return Vector2<_T>(c[0] + (_T)v[0],c[1] + (_T)v[1]);
	}

	template<class _T2> Vector2<_T> operator -(const Vector2<_T2>& v) const
	{
		return Vector2<_T>(c[0] - (_T)v[0],c[1] - (_T)v[1]);
	}

	template<class _T2> Vector2<_T> operator &(const Vector2<_T2>& v) const
	{
		return Vector2<_T>(c[0] * (_T)v[0],c[1] * (_T)v[1]);
	}

	template<class _T2> Vector2<_T> operator |(const Vector2<_T2>& v) const
	{
		return Vector2<_T>(c[0] / (_T)v[0],c[1] / (_T)v[1]);
	}

	template<class _T2> _T operator *(const Vector2<_T2>& v) const
	{
		return _T(c[0] * (_T)v[0] + c[1] * (_T)v[1]);
	}

	template<class _T2> _T operator %(const Vector2<_T2>& v) const
	{
		return _T(c[0] * (_T)v[1] - c[0] * (_T)v[1]);
	}

	_T& operator!() const
	{
		return c[0]*c[0]+c[1]*c[1];
	}

	Vector2<_T>& operator-() const
	{
		return Vector2<_T>(-c[0],-c[1]);
	}

};

}

#endif
