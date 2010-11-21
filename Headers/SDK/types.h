/********************************************************/
/* FILE: types.h                                        */
/* DESCRIPTION: Declares basic types for HAGE.          */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include "preproc.h"

namespace HAGE {

typedef float f32;
typedef double f64;

#ifdef COMPILER_MSVC
    typedef unsigned __int64 u64;
    typedef signed __int64 i64;
    typedef unsigned long u32;
    typedef signed long i32;
#elif defined(COMPILER_GCC)
    typedef unsigned long long u64;
    typedef signed long long i64;
    typedef unsigned int u32;
    typedef signed int i32;
#else
    #error UNKNOWN COMPILER
#endif

typedef unsigned short u16;
typedef signed short i16;
typedef unsigned char u8;
typedef signed char i8;

#ifdef TARGET_X64
    #ifdef COMPILER_MSVC
        typedef unsigned __int64 up;
        typedef signed __int64 ip;
    #else
        typedef unsigned long long up;
        typedef signed long long ip;
    #endif
#elif defined(TARGET_X86)
    typedef unsigned long up;
    typedef signed long ip;
#else
    #error UNKNOWN TARGET
#endif

typedef unsigned int result;

typedef struct _GUID
{
	union
	{
        u64 ll[2];
        u32 l[4];
        u16 s[8];
        u8 c[16];
	};

	bool operator ==(const _GUID& other) const
	{
		return ll[0]==other.ll[0] && ll[1] == other.ll[1];
	}
	bool operator !=(const _GUID& other) const
	{
		return ll[0]!=other.ll[0] || ll[1] != other.ll[1];
	}
	bool operator <(const _GUID& other) const
	{
		return ll[0] < other.ll[0] || ( ll[0] == other.ll[0] && ll[1] < other.ll[1] );
	}
	bool operator >(const _GUID& other) const
	{
		return ll[0] > other.ll[0] || ( ll[0] == other.ll[0] && ll[1] > other.ll[1] );
	}
	bool operator <=(const _GUID& other) const
	{
		return ll[0] < other.ll[0] || ( ll[0] == other.ll[0] && ll[1] <= other.ll[1] );
	}
	bool operator >=(const _GUID& other) const
	{
		return ll[0] > other.ll[0] || ( ll[0] == other.ll[0] && ll[1] >= other.ll[1] );
	}
} guid;

#ifdef COMPILER_MSVC
#define DECLARE_GUID_WIN(x,l1,s1,s2,b1,b2,b3,b4,b5,b6,b7,b8) static const guid guid##x = {{{ ( ((u64)(l1)<<32)|((u64)(s1)<<16)|((u64)(s2)<<0) ) , \
	(((u64)(b1)<<56)|((u64)(b2)<<48)|((u64)(b3)<<40)|((u64)(b4)<<32)|((u64)(b5)<<24)|((u64)(b6)<<16)|((u64)(b7)<<8)|((u64)(b8)<<0)) }}}
#define DECLARE_GUID(x,l1,s1,s2,s3,x1) static const guid guid##x = {{{ ( ((u64)(l1)<<32)|((u64)(s1)<<16)|((u64)(s2)<<0) ) , \
	(((u64)(s3)<<48)|((u64)(x1)<<0)) }}}
#elif defined(COMPILER_GCC)
#define DECLARE_GUID_WIN(x,l1,s1,s2,b1,b2,b3,b4,b5,b6,b7,b8) static const guid guid##x = {{{ ( ((u64)(l1)<<32)|((u64)(s1)<<16)|((u64)(s2)<<0) ) , \
	(((u64)(b1)<<56)|((u64)(b2)<<48)|((u64)(b3)<<40)|((u64)(b4)<<32)|((u64)(b5)<<24)|((u64)(b6)<<16)|((u64)(b7)<<8)|((u64)(b8)<<0)) }}}
#define DECLARE_GUID(x,l1,s1,s2,s3,x1) static const guid guid##x = {{{ ( ((u64)(l1)<<32)|((u64)(s1)<<16)|((u64)(s2)<<0) ) , \
	(((u64)(s3)<<48)|((u64)(x1##LL)<<0)) }}}
#endif

DECLARE_GUID_WIN(Null,0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

const int THREAD_MODE_ST = 0x00000001;
const int THREAD_MODE_MT = 0x00000002;

class MemHandle
{
public:
	MemHandle() : _p(nullptr) {}
	~MemHandle(){}

	bool isValid() {return _p!=nullptr;}
private:
	MemHandle(void* p) : _p(p) {}
	void* _p;

	friend class PinBase;
};

}

#endif
