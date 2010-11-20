#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __RESULT_H__
#define __RESULT_H__

#include "types.h"

namespace HAGE {


// results

enum {
	S_OK = 0x00000000,
	E_FAIL = 0x80000000,
	RESERVED = 0xffffffff
};

inline bool failed(result r)
{
	return (r&E_FAIL)!=0;
}

inline bool succeeded(result r)
{
	return !failed(r);
}

}

#endif
