/********************************************************/
/* FILE: interlocked_functions.h                        */
/* DESCRIPTION: None                                    */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef INTERLOCKED_FUNCTIONS_H_INCLUDED
#define INTERLOCKED_FUNCTIONS_H_INCLUDED

#if defined(COMPILER_MSVC)
#include <boost/detail/interlocked.hpp>
#elif defined(COMPILER_GCC)

#include "types.h"

inline HAGE::u32 _InterlockedIncrement(volatile HAGE::u32* p)
{
    return __sync_add_and_fetch(p,1);
}

inline HAGE::u64 _InterlockedIncrement(volatile HAGE::u64* p)
{
    return __sync_add_and_fetch(p,1);
}

inline HAGE::i32 _InterlockedIncrement(volatile HAGE::i32* p)
{
    return __sync_add_and_fetch(p,1);
}

inline HAGE::i64 _InterlockedIncrement(volatile HAGE::i64* p)
{
    return __sync_add_and_fetch(p,1);
}

inline HAGE::u32 _InterlockedDecrement(volatile HAGE::u32* p)
{
    return __sync_sub_and_fetch(p,1);
}

inline HAGE::u64 _InterlockedDecrement(volatile HAGE::u64* p)
{
    return __sync_sub_and_fetch(p,1);
}

inline HAGE::i32 _InterlockedDecrement(volatile HAGE::i32* p)
{
    return __sync_sub_and_fetch(p,1);
}

inline HAGE::i64 _InterlockedDecrement(volatile HAGE::i64* p)
{
    return __sync_sub_and_fetch(p,1);
}

#else
#error UNSUPPORTED COMPILER
#endif


#endif // INTERLOCKED_FUNCTIONS_H_INCLUDED
