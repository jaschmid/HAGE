/********************************************************/
/* FILE: HageMath.h                                     */
/* DESCRIPTION: Defines common math structures          */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef HAGE_MATH_H_INCLUDED
#define HAGE_MATH_H_INCLUDED

#include "types.h"

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

	Vector3(){};

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
		return _T(c[0] * (_T)v[0] + c[1] * (_T)v[1] + c[2] * (_T)v[2]);
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
};

template<typename _T = f32> struct Vector4
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

	Vector4(const Vector3<>& _v,const _T& _4) : x(_v.x),y(_v.y),z(_v.z),w(_4)
	{
	}

	template<class _T2> Vector4(const _T2& _1,const _T2& _2,const _T2& _3,const _T2& _4) : x((const _T&)_1),y((const _T&)_2),z((const _T&)_3),w((const _T&)_4)
	{
	}

	Vector4()
	{
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

template<class _T = f32> struct Matrix4
{
	union
	{
		_T			v[16];
	};

	// constructors

	Matrix4()
	{
	}

	template<class _T2> Matrix4(const Matrix4<_T2>& other)
	{
		for(int i=0;i<16;++i)
			v[i]=(_T)other.v[i];
	}

	template<class _T2> Matrix4(const Vector4<_T2>& r1,const Vector4<_T2>& r2,const Vector4<_T2>& r3,const Vector4<_T2>& r4)
	{
		Row(0)=(Vector4<_T>)r1;
		Row(1)=(Vector4<_T>)r2;
		Row(2)=(Vector4<_T>)r3;
		Row(3)=(Vector4<_T>)r4;
	}

	static const Matrix4<_T> One()
	{
		return Matrix4(
			Vector4<_T>(1.0f,0.0f,0.0f,0.0f),
			Vector4<_T>(0.0f,1.0f,0.0f,0.0f),
			Vector4<_T>(0.0f,0.0f,1.0f,0.0f),
			Vector4<_T>(0.0f,0.0f,0.0f,1.0f)
		);
	}

	static const Matrix4<_T> Zero()
	{
		return Matrix4(
			Vector4<_T>(0.0f,0.0f,0.0f,0.0f),
			Vector4<_T>(0.0f,0.0f,0.0f,0.0f),
			Vector4<_T>(0.0f,0.0f,0.0f,0.0f),
			Vector4<_T>(0.0f,0.0f,0.0f,0.0f)
		);
	}

	static const Matrix4<_T> Translate(const Vector3<>& v)
	{
		return Matrix4(
			Vector4<_T>(1.0f,0.0f,0.0f,v.x),
			Vector4<_T>(0.0f,1.0f,0.0f,v.y),
			Vector4<_T>(0.0f,0.0f,1.0f,v.z),
			Vector4<_T>(0.0f,0.0f,0.0f,1.0f)
		);
	}

	static const Matrix4<_T> Scale(const Vector3<>& v)
	{
		return Matrix4(
			Vector4<_T>(v.x ,0.0f,0.0f,0.0f),
			Vector4<_T>(0.0f,v.y ,0.0f,0.0f),
			Vector4<_T>(0.0f,0.0f,v.z ,0.0f),
			Vector4<_T>(0.0f,0.0f,0.0f,1.0f)
		);
	}
	static const Matrix4<_T> AngleRotation(const Vector3<_T>& a,const _T& angle)
	{
		_T _cos = cos(angle);
		_T _1cos = 1.0f - _cos;
		_T _sin = sin(angle);
		return Matrix4(
			Vector4<_T>(_cos + a.x*a.x*_1cos ,		a.x*a.y*_1cos - a.z*_sin,	a.x*a.z*_1cos + a.y*_sin,	0.0f),
			Vector4<_T>(a.y*a.x*_1cos + a.z*_sin,	_cos + a.y*a.y*_1cos,		a.y*a.z*_1cos - a.x*_sin,	0.0f),
			Vector4<_T>(a.z*a.x*_1cos - a.y*_sin,	a.z*a.y*_1cos + a.x*_sin,	a.z*a.z*_1cos + _cos,		0.0f),
			Vector4<_T>(0.0f,						0.0f,						0.0f,						1.0f)
		);
	}

	static const Matrix4<_T> Perspective(const _T& near,const _T& far,const _T& fovx,const _T& fovy)
	{
		_T h,w,Q;
		w = (_T)1/tan(fovx*0.5f);
		h = (_T)1/tan(fovy*0.5f);
		Q = far/(far-near);
		return Matrix4(
			Vector4<_T>(w,			0.0f,	0.0f,			0.0f),
			Vector4<_T>(0.0f,		h,		0.0f,			0.0f),
			Vector4<_T>(0.0f,		0.0f,	Q,				-Q*near),
			Vector4<_T>(0.0f,		0.0f,	1,				0.0f)
		);
	}

	// basic operations

	template<class _T2> const Matrix4<_T> operator *(const Matrix4<_T2>& other) const
	{
		Matrix4<_T> res;
		for(int ir =0;ir<4;++ir)
			for(int ic =0;ic<4;++ic)
				res.v[ir*4+ic]=Row(ir)*other.Column(ic);
		return res;
	}

	template<class _T2> const Vector4<_T> operator *(const Vector4<_T2>& other) const
	{
		Vector4<_T> res;
		for(int ir =0;ir<4;++ir)
			res[ir]=Row(ir)*other;
		return res;
	}

	// access operators
	Vector4<_T>& Row(const u32& i)
	{
		return *(Vector4<_T>*)&v[i*4];
	}
	const Vector4<_T>& Row(const u32& i) const
	{
		return *(const Vector4<_T>*)&v[i*4];
	}
	const Vector4<_T> Column(const u32& i) const
	{
		return Vector4<_T>(v[i],v[4+i],v[8+i],v[12+i]);
	}

};

}

#endif
