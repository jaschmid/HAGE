/********************************************************/
/* FILE: Vector3.h                                      */
/* DESCRIPTION: Hage Vector3 class                      */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"
#include "Vector2.h"

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef HAGE_MATH_VECTOR3_H_INCLUDED
#define HAGE_MATH_VECTOR3_H_INCLUDED

namespace HAGE {



template<typename _T = f32> struct Vector3
{
public:
	union
	{
		_T c[3];
		struct
		{
			_T x,y,z;
		};
	};

	// access functions

	Vector2<_T>& xy()
	{
		return *(Vector2<_T>*)&c[0];
	}
	Vector2<_T>& yz()
	{
		return *(Vector2<_T>*)&c[1];
	}
	_T& operator[](const up& i)
	{
		return c[i];
	}

	// const access functions

	const Vector2<_T>& xy() const
	{
		return *(const Vector2<_T>*)&c[0];
	}
	const Vector2<_T>& yz() const
	{
		return *(const Vector2<_T>*)&c[1];
	}
	const Vector2<_T>& xz() const
	{
		return Vector2<_T>(c[0],c[1]);
	}
	const _T& operator[](const up& i) const
	{
		return c[i];
	}

	// constructors

	Vector3(const _T& _1,const _T& _2,const _T& _3) : x(_1),y(_2),z(_3)
	{
	}

	template<class _T2> Vector3(const _T2& _1,const _T2& _2,const _T2& _3) : x((const _T&)_1),y((const _T&)_2),z((const _T&)_3)
	{
	}

	template<class _T2> Vector3(const Vector2<_T2>& _v,const _T2& _3) : x((const _T&)_v.x),y((const _T&)_v.y),z((const _T&)_3)
	{
	}

	template<class _T2> Vector3(const _T2& _1,const Vector2<_T2>& _v) : x((const _T&)_1),y((const _T&)_v.x),z((const _T&)_v.y)
	{
	}
	
	template<class _T2> Vector3(const Vector3<_T2>& _v) : x((const _T&)_v.x),y((const _T&)_v.y),z((const _T&)_v.z)
	{
	}

	Vector3(){};

	//comparison

	bool operator ==(const Vector3<_T>& v2)
	{
		return (x==v2.x) && (y==v2.y) && (z==v2.z);
	}
	
	template<class _T2> Vector3<_T>& operator = (const Vector3<_T2>& _v) 
	{
		x = ((const _T&)_v.x);
		y = ((const _T&)_v.y);
		z = ((const _T&)_v.z);
		return *this;
	}

	// math operations
	// scalar

	template<class _T2> Vector3<_T> operator *(const _T2& v) const
	{
		return Vector3<_T>(c[0] * (_T)v,c[1] * (_T)v,c[2] * (_T)v);
	}

	template<class _T2> Vector3<_T> operator /(const _T2& v) const
	{
		return Vector3<_T>(c[0] / (_T)v,c[1] / (_T)v,c[2] / (_T)v);
	}

	// vector

	template<class _T2> Vector3<_T> operator +(const Vector3<_T2>& v) const
	{
		return Vector3<_T>(c[0] + (_T)v[0],c[1] + (_T)v[1], c[2] + (_T)v[2]);
	}

	template<class _T2> Vector3<_T> operator -(const Vector3<_T2>& v) const
	{
		return Vector3<_T>(c[0] - (_T)v[0],c[1] - (_T)v[1], c[2] - (_T)v[2]);
	}

	template<class _T2> Vector3<_T> operator &(const Vector3<_T2>& v) const
	{
		return Vector3<_T>(c[0] * (_T)v[0],c[1] * (_T)v[1], c[2] * (_T)v[2]);
	}

	template<class _T2> Vector3<_T> operator |(const Vector3<_T2>& v) const
	{
		return Vector3<_T>(c[0] / (_T)v[0],c[1] / (_T)v[1], c[2] / (_T)v[2]);
	}

	template<class _T2> Vector3<_T> operator +=(const Vector3<_T2>& v)
	{
		c[0]+=v.c[0];c[1]+=v.c[1];c[2]+=v.c[2];
		return *this;
	}
	template<class _T2> Vector3<_T> operator -=(const Vector3<_T2>& v)
	{
		c[0]-=v.c[0];c[1]-=v.c[1];c[2]-=v.c[2];
		return *this;
	}
	template<class _T2> Vector3<_T> operator &=(const Vector3<_T2>& v)
	{
		c[0]*=v.c[0];c[1]*=v.c[1];c[2]*=v.c[2];
		return *this;
	}
	template<class _T2> Vector3<_T> operator |=(const Vector3<_T2>& v)
	{
		c[0]/=v.c[0];c[1]/=v.c[1];c[2]/=v.c[2];
		return *this;
	}

	template<class _T2> _T operator *(const Vector3<_T2>& v) const
	{
		return (_T)(c[0] * (_T)v[0] + c[1] * (_T)v[1] + c[2] * (_T)v[2]);
	}

	template<class _T2> Vector3<_T> operator %(const Vector3<_T2>& v) const
	{
		return Vector3<_T>(c[1] * (_T)v[2] - c[2] * (_T)v[1], c[2] * (_T)v[0] - c[0] * (_T)v[2], c[0] * (_T)v[1] - c[1] * (_T)v[0]);
	}

	_T operator!() const
	{
		return c[0]*c[0]+c[1]*c[1]+c[2]*c[2];
	}

	Vector3<_T> operator-() const
	{
		return Vector3<_T>(-c[0],-c[1],-c[2]);
	}

	Vector3<_T> normalize() const
	{
		return (*this)/(sqrtf(!(*this)));
	}

	float length() const
	{
		return sqrtf(!(*this));
	}

	float sqLength() const
	{
		return !(*this);
	}
};



}

#endif
