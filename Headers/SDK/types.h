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

typedef struct _HAGETime
{
	_HAGETime operator -(const _HAGETime& other) const
	{
		_HAGETime ret = {time_utc - other.time_utc};
		return ret;
	}
	_HAGETime operator +(const _HAGETime& other) const
	{
		_HAGETime ret = {time_utc + other.time_utc};
		return ret;
	}
	_HAGETime operator -=(const _HAGETime& other)
	{
		time_utc -= other.time_utc;
		_HAGETime ret = {time_utc};
		return ret;
	}
	_HAGETime operator +=(const _HAGETime& other)
	{
		time_utc += other.time_utc;
		_HAGETime ret = {time_utc};
		return ret;
	}	
	bool operator ==(const _HAGETime& other) const
	{
		return time_utc == other.time_utc;
	}	
	bool operator !=(const _HAGETime& other) const
	{
		return time_utc != other.time_utc;
	}
	bool operator >(const _HAGETime& other) const
	{
		return time_utc > other.time_utc;
	}	
	bool operator <(const _HAGETime& other) const
	{
		return time_utc < other.time_utc;
	}
	// precise up to microseconds
	f32 toSeconds()
	{
		return static_cast<f32>(time_utc / 1000)/1000000.0f;
	}
	// full precision
	f64 toSecondsDouble()
	{
		return static_cast<f64>(time_utc)/1000000000.0;
	}
	// time elapsed in nanoseconds (1/1000000000 seconds)
	u64 time_utc;
} t64;

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

template<class _C> class guid_of
{
};

#define DECLARE_GUID_WIN(x,l1,s1,s2,b1,b2,b3,b4,b5,b6,b7,b8) const guid guid##x = {{{ ( ((u64)(l1)<<32)|((u64)(s1)<<16)|((u64)(s2)<<0) ) , \
	(((u64)(b1)<<56)|((u64)(b2)<<48)|((u64)(b3)<<40)|((u64)(b4)<<32)|((u64)(b5)<<24)|((u64)(b6)<<16)|((u64)(b7)<<8)|((u64)(b8)<<0)) }}}
#define DECLARE_GUID(x,l1,s1,s2,s3,x1) const guid guid##x = {{{ ( ((u64)(l1)<<32)|((u64)(s1)<<16)|((u64)(s2)<<0) ) , \
	(((u64)(s3)<<48)|((u64)(x1)<<0)) }}}
#define DECLARE_CLASS_GUID_EX(x,l1,s1,s2,s3,x1,c) DECLARE_GUID(x,l1,s1,s2,s3,x1); template<> class guid_of<c> { public: static const guid& Get(){return guid##x;} static const char* Name(){return #x;} }
#define DECLARE_CLASS_GUID(x,l1,s1,s2,s3,x1) DECLARE_CLASS_GUID_EX(x,l1,s1,s2,s3,x1,x)
#define DEFINE_CLASS_GUID(x)
#define DEFINE_CLASS_GUID_EX(x,c)


DECLARE_CLASS_GUID_EX(Null,0x00000000, 0x0000, 0x0000, 0x0000, 0x000000000000,void);

//constants

static const int THREAD_MODE_ST = 0x00000001;
static const int THREAD_MODE_MT = 0x00000002;


static const u64 SYSTEM_TIME_TICKS_PER_SECOND = 1000000000LL;
static const u64 RESOURCE_DOMAIN_MAX_TIME_SLICE	= SYSTEM_TIME_TICKS_PER_SECOND / 2;

class MemHandle
{
public:
	MemHandle() : _p(nullptr) {}
	~MemHandle(){}

	bool isValid() const {return _p!=(void*)nullptr;}
private:
	MemHandle(void* p) : _p(p) {}
	void* _p;

	friend class PinBase;
};

}

#endif
