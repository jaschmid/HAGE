/********************************************************/
/* FILE: Quaternion.h                                   */
/* DESCRIPTION: Hage Quaternion class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"
#include "Vector3.h"

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef HAGE_MATH_QUATERNION_H_INCLUDED
#define HAGE_MATH_QUATERNION_H_INCLUDED

namespace HAGE {

	
template<class _T> struct Quaternion
{
public:

	union
	{
		_T c[4];
		struct
		{
			_T r,i,j,k;
		};
	};

	Quaternion(const _T& _1,const _T& _2,const _T& _3,const _T& _4) :
		r(_1),i(_2),j(_3),k(_4)
	{
	}

	Quaternion(const _T& _1, const Vector3<_T>& _2) :
		r(_1),i(_2.x),j(_2.y),k(_2.z)
	{
	}

	//assignment

	Quaternion& operator = (const Quaternion<_T>& _1)
	{
		r = _1.r;
		i = _1.i;
		j = _1.j;
		k = _1.k;
		return *this;
	}

	//comparison

	bool operator ==(const Quaternion<_T>& _1) const
	{
		return r == _1.r && i == _1.i && j == _1.j && k == _1.k;
	}
	bool operator !=(const Quaternion<_T>& _1) const
	{
		return r != _1.r || i != _1.i || j != _1.j || k != _1.k;
	}

	//basic init and test

	static Quaternion<_T> Zero()
	{
		return Quaternion<_T>(Zero<_T>(),Zero<_T>(),Zero<_T>(),Zero<_T>());
	}
	static Quaternion<_T> One()
	{
		return Quaternion<_T>(One<_T>(),Zero<_T>(),Zero<_T>(),Zero<_T>());
	}
	static Quaternion<_T> NaN()
	{
		return Quaternion<_T>(NaN<_T>(),NaN<_T>(),NaN<_T>(),NaN<_T>());
	}

	bool IsZero() const
	{
		return *this == Quaternion<_T>::Zero();
	}
	bool IsOne() const
	{
		return *this == Quaternion<_T>::One();
	}
	bool IsNaN() const
	{
		return IsNan<_T>(r) || IsNan<_T>(i)  || IsNan<_T>(j) || IsNan<_T>(k) ;
	}

	// math operations

	Quaternion<_T> operator - () const
	{
		return Quaternion<_T>(-r,i,j,k);
	}

	Quaternion<_T> conjugate() const
	{
		return Quaternion<_T>(r,-i,-j,-k);
	}

	Quaternion<_T> operator ~ () const
	{
		return conjugate();
	}

	_T sqMagnitude() const
	{
		return r*r + i*i + j*j + k*k;
	}

	_T operator !() const
	{
		return sqMagnitude();
	}

	_T magnitude() const
	{
		return sqrt(sqMagnitude());
	}

	Quaternion<_T> operator + (const Quaternion<_T>& _1) const
	{
		return Quaternion<_T>(r+_1.r,i+_1.i,j+_1.j,k+_1.k);
	}

	Quaternion<_T> operator - (const Quaternion<_T>& _1) const
	{
		return Quaternion<_T>(r-_1.r,i-_1.i,j-_1.j,k-_1.k);
	}

};


}

#endif