/********************************************************/
/* FILE: Vector4.h                                      */
/* DESCRIPTION: Hage Vector4 class                      */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"
#include "Vector2.h"
#include "Vector3.h"

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef HAGE_MATH_VECTOR4_H_INCLUDED
#define HAGE_MATH_VECTOR4_H_INCLUDED

namespace HAGE {


template<typename _T> struct Vector4
{
public:
	union
	{
		_T c[4];
		struct
		{
			_T x,y,z,w;
		};
	};

	// access functions

	Vector3<_T>& xyz()
	{
		return *(Vector3<_T>*)&c[0];
	}
	Vector3<_T>& yzw()
	{
		return *(Vector3<_T>*)&c[1];
	}
	_T& operator[](const up& i)
	{
		return c[i];
	}

	// const access functions

	const Vector3<_T>& xyz() const
	{
		return *(const Vector3<_T>*)&c[0];
	}
	const Vector3<_T>& yzw() const
	{
		return *(const Vector3<_T>*)&c[1];
	}
	const _T& operator[](const up& i) const
	{
		return c[i];
	}

	// constructors

	Vector4(const _T& _1,const _T& _2,const _T& _3,const _T& _4) : x(_1),y(_2),z(_3),w(_4)
	{
	}

	template<class _T2> Vector4(const Vector3<_T2>& _v,const _T& _4) : x((_T)_v.x),y((_T)_v.y),z((_T)_v.z),w(_4)
	{
	}
	
	template<class _T2> Vector4(const Vector4<_T2>& _v) : x((_T)_v.x),y((_T)_v.y),z((_T)_v.z),w((_T)_v.w)
	{
	}

	template<class _T2> Vector4(const _T2& _1,const _T2& _2,const _T2& _3,const _T2& _4) : x((const _T&)_1),y((const _T&)_2),z((const _T&)_3),w((const _T&)_4)
	{
	}

	Vector4()
	{
	}

	//assignment

	
	template<class _T2> Vector4<_T>& operator = (const Vector4<_T2>& _v) 
	{
		x = ((const _T&)_v.x);
		y = ((const _T&)_v.y);
		z = ((const _T&)_v.z);
		w = ((const _T&)_v.w);
		return *this;
	}

	// math operations
	// scalar

	template<class _T2> Vector4<_T> operator *(const _T2& v) const
	{
		return Vector4<_T>(c[0] * (_T)v,c[1] * (_T)v,c[2] * (_T)v, c[3] * (_T)v);
	}

	template<class _T2> Vector4<_T> operator /(const _T2& v) const
	{
		return Vector4<_T>(c[0] / (_T)v,c[1] / (_T)v,c[2] / (_T)v, c[3] / (_T)v);
	}

	// vector

	template<class _T2> Vector4<_T> operator +(const Vector4<_T2>& v) const
	{
		return Vector4<_T>(c[0] + (_T)v[0],c[1] + (_T)v[1], c[2] + (_T)v[2], c[3] + (_T)v[3]);
	}

	template<class _T2> Vector4<_T> operator -(const Vector4<_T2>& v) const
	{
		return Vector4<_T>(c[0] - (_T)v[0],c[1] - (_T)v[1], c[2] - (_T)v[2], c[3] - (_T)v[3]);
	}

	template<class _T2> Vector4<_T> operator &(const Vector4<_T2>& v) const
	{
		return Vector4<_T>(c[0] * (_T)v[0],c[1] * (_T)v[1], c[2] * (_T)v[2], c[3] * (_T)v[3]);
	}

	template<class _T2> Vector4<_T> operator |(const Vector4<_T2>& v) const
	{
		return Vector4<_T>(c[0] / (_T)v[0],c[1] / (_T)v[1], c[2] / (_T)v[2], c[3] / (_T)v[3]);
	}

	template<class _T2> _T operator *(const Vector4<_T2>& v) const
	{
		return _T(c[0] * (_T)v[0] + c[1] * (_T)v[1] + c[2] * (_T)v[2] + c[3] * (_T)v[3]);
	}

	_T& operator!() const
	{
		return c[0]*c[0]+c[1]*c[1]+c[2]*c[2]+c[3]*c[3];
	}

	Vector3<_T>& operator-() const
	{
		return Vector3<_T>(-c[0],-c[1],-c[2],-c[3]);
	}
};

}

#endif
