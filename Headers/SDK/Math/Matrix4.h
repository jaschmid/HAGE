/********************************************************/
/* FILE: Matrix4.h                                      */
/* DESCRIPTION: Hage 4x4 Matrix class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef HAGE_MATH_MATRIX4_H_INCLUDED
#define HAGE_MATH_MATRIX4_H_INCLUDED

namespace HAGE {

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
	

	static const Matrix4<_T> NaN()
	{
		_T nan = HAGE::NaN<_T>();
		return Matrix4(
			Vector4<_T>(nan,nan,nan,nan),
			Vector4<_T>(nan,nan,nan,nan),
			Vector4<_T>(nan,nan,nan,nan),
			Vector4<_T>(nan,nan,nan,nan)
		);
	}

	static const Matrix4<_T> OuterProduct(const Vector4<_T>& v1,const Vector4<_T>& v2)
	{
		Matrix4<_T> result;

		for(int ir = 0; ir < 4; ir ++)
			for(int ic = 0; ic < 4; ic ++)
				result.v[ir*4+ic] = v1.c[ir] * v2.c[ic];

		return result;
	}

	static const Matrix4<_T> Translate(const Vector3<_T>& v)
	{
		return Matrix4(
			Vector4<_T>(1.0f,0.0f,0.0f,v.x),
			Vector4<_T>(0.0f,1.0f,0.0f,v.y),
			Vector4<_T>(0.0f,0.0f,1.0f,v.z),
			Vector4<_T>(0.0f,0.0f,0.0f,1.0f)
		);
	}

	static const Matrix4<_T> LookAt(const Vector3<_T>& eye,const Vector3<_T>& at,const Vector3<_T>& up)
	{
		Vector3<> zaxis = (at - eye).normalize();
		Vector3<> xaxis = (up % zaxis).normalize();
		Vector3<> yaxis = (zaxis % xaxis);
		return Matrix4(
			Vector4<_T>(xaxis,-(xaxis*eye)),
			Vector4<_T>(yaxis,-(yaxis*eye)),
			Vector4<_T>(zaxis,-(zaxis*eye)),
			Vector4<_T>(0.0f,0.0f,0.0f,1.0f)
		);
	}

	static const Matrix4<_T> Scale(const Vector3<_T>& v)
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

	// basic operations

	template<class _T2> const Matrix4<_T> operator *(const Matrix4<_T2>& other) const
	{
		Matrix4<_T> res;
		for(int ir =0;ir<4;++ir)
			for(int ic =0;ic<4;++ic)
				res.v[ir*4+ic]=Row(ir)*other.Column(ic);
		return res;
	}

	
	template<class _T2> Matrix4<_T>& operator *=(const Matrix4<_T2>& other)
	{
		Matrix4<_T> copy = *this;
		for(int ir =0;ir<4;++ir)
			for(int ic =0;ic<4;++ic)
				v[ir*4+ic]=copy.Row(ir)*other.Column(ic);
		return *this;
	}

	const Matrix4<_T> operator *(const _T& other) const
	{
		Matrix4<_T> res;
		for(int ir =0;ir<4;++ir)
			for(int ic =0;ic<4;++ic)
				res.v[ir*4+ic]=v[ir*4+ic]*other;
		return res;
	}

	
	Matrix4<_T>& operator *=(const _T& other)
	{
		Matrix4<_T> copy = *this;
		for(int ir =0;ir<4;++ir)
			for(int ic =0;ic<4;++ic)
				v[ir*4+ic]*=other;
		return *this;
	}
	
	template<class _T2> const Matrix4<_T> operator +(const Matrix4<_T2>& other) const
	{
		Matrix4<_T> res;
		for(int ir =0;ir<4;++ir)
			for(int ic =0;ic<4;++ic)
				res.v[ir*4+ic]=v[ir*4+ic] + other.v[ir*4+ic];
		return res;
	}

	
	template<class _T2> Matrix4<_T>& operator +=(const Matrix4<_T2>& other)
	{
		for(int ir =0;ir<4;++ir)
			for(int ic =0;ic<4;++ic)
				v[ir*4+ic]+=other.v[ir*4+ic];
		return *this;
	}

	template<class _T2> const Vector4<_T> operator *(const Vector4<_T2>& other) const
	{
		Vector4<_T> res;
		for(int ir =0;ir<4;++ir)
			res[ir]=Row(ir)*other;
		return res;
	}

	//status operators

	bool IsNaN() const
	{
		return HAGE::IsNaN(v[0]);
	}

	operator bool() const
	{
		return IsNaN();
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

	const Matrix4<_T> Transpose() const
	{
		return Matrix4(
			Vector4<_T>(v[0],v[4],v[8], v[12]),
			Vector4<_T>(v[1],v[5],v[9], v[13]),
			Vector4<_T>(v[2],v[6],v[10],v[14]),
			Vector4<_T>(v[3],v[7],v[11],v[15])
		);
	}

	// advanced operations

	//Invert code from Intel paper Streaming SIMD Extensions - Inverse of 4x4 Matrix (Ref AP-928)
	const Matrix4<_T> Invert(_T* pDet = nullptr) const
	{
		_T tmp[12];
		_T det;
		Matrix4<_T> result;
		const Matrix4<_T> transpose = Transpose();

		const _T* src = transpose.v;
		_T* dst = result.v;

		/* calculate pairs for first 8 elements (cofactors) */
		tmp[0]  = src[10] * src[15];
		tmp[1]  = src[11] * src[14];
		tmp[2]  = src[9]  * src[15];
		tmp[3]  = src[11] * src[13];
		tmp[4]  = src[9]  * src[14];
		tmp[5]  = src[10] * src[13];
		tmp[6]  = src[8]  * src[15];
		tmp[7]  = src[11] * src[12];
		tmp[8]  = src[8]  * src[14];
		tmp[9]  = src[10] * src[12];
		tmp[10] = src[8]  * src[13];
		tmp[11] = src[9]  * src[12];

		/* calculate first 8 elements (cofactors) */
		dst[0]  = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
		dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
		dst[1]  = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
		dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
		dst[2]  = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
		dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
		dst[3]  = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
		dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];

		
		/* calculate determinant */
		det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

		if(pDet)
			*pDet = det;
		//return nan if determinant is zero
		if(IsZero<_T>(det))
			return Matrix4<_T>::NaN();

		det = 1/det;

		dst[4]  = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
		dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
		dst[5]  = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
		dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
		dst[6]  = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
		dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
		dst[7]  = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
		dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];

		 /* calculate pairs for second 8 elements (cofactors) */
		tmp[0]  = src[2]*src[7];
		tmp[1]  = src[3]*src[6];
		tmp[2]  = src[1]*src[7];
		tmp[3]  = src[3]*src[5];
		tmp[4]  = src[1]*src[6];
		tmp[5]  = src[2]*src[5];
		tmp[6]  = src[0]*src[7];
		tmp[7]  = src[3]*src[4];
		tmp[8]  = src[0]*src[6];
		tmp[9]  = src[2]*src[4];
		tmp[10] = src[0]*src[5];
		tmp[11] = src[1]*src[4];

		/* calculate second 8 elements (cofactors) */
		dst[8]  = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
		dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
		dst[9]  = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
		dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
		dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
		dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
		dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
		dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
		dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
		dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
		dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
		dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
		dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
		dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
		dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
		dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
		
		/* calculate matrix inverse */
		return result*det;
	}

	const _T Determinant() const
	{
		_T tmp[12];
		_T det;
		Matrix4<_T> result;
		const Matrix4<_T> transpose = Transpose();

		const _T* src = transpose.v;
		_T* dst[4];

		/* calculate pairs for first 8 elements (cofactors) */
		tmp[0]  = src[10] * src[15];
		tmp[1]  = src[11] * src[14];
		tmp[2]  = src[9]  * src[15];
		tmp[3]  = src[11] * src[13];
		tmp[4]  = src[9]  * src[14];
		tmp[5]  = src[10] * src[13];
		tmp[6]  = src[8]  * src[15];
		tmp[7]  = src[11] * src[12];
		tmp[8]  = src[8]  * src[14];
		tmp[9]  = src[10] * src[12];
		tmp[10] = src[8]  * src[13];
		tmp[11] = src[9]  * src[12];

		/* calculate first 8 elements (cofactors) */
		dst[0]  = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
		dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
		dst[1]  = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
		dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
		dst[2]  = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
		dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
		dst[3]  = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
		dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];

		/* calculate determinant */
		det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

		return det;
	}

	// Originally based on public domain code by <Ajay_Shah@rand.org>
	// which can be found at http://lib.stat.cmu.edu/general/ajay
	/*
	  Inputs:
	  N, integer
	  A, N x N matrix _indexed_from_1_
	  Returns:
	  U, N x N matrix, _indexed_from_1_, you must allocate before
	  calling this routine,
	  nullity, integer
	*/

	const Matrix4<_T> Cholesky() const
	{
	  Matrix4<_T> U = Matrix4<_T>::Zero();
	  Matrix4<_T> A = *this;
	  static const _T TOOSMALL ((_T) 0.0);
	  static const int N = 4;
	  int nullity = 0;
	  int row, j, k;
	  _T sum;

	  for (row=0; row<N; row++) {
		sum = A.v[row*4+row];
		for (j=0; j<=(row-1); j++) sum -= U.v[j*4+row]*U.v[j*4+row];
		if (sum > TOOSMALL) {
		  U.v[row*4+row] = sqrt(sum);
		  for (k=(row+1); k<N; k++) {
			sum = A.v[row*4+k];
			for (j=0; j<=(row-1); j++)
			  sum -= U.v[j*4+row]*U.v[j*4+k];
			U.v[row*4+k] = sum/U.v[row*4+row];
		  }
		}
		else { 
		  for (k=row; k<N; k++) U.v[row*4+k] = 0.0;
		  nullity++;
		}
	  }

	  if(nullity != 0)
		  return Matrix4<_T>::NaN();

	  return U;
	}
};


}

#endif
