#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef DOMAINMEMORY_INCLUDED_H__
#define DOMAINMEMORY_INCLUDED_H__

#include "HAGE.h"

namespace HAGE {

class DomainMemory
{
public:
	DomainMemory();
	~DomainMemory();

	void* Allocate(u64 size);
	void Free(void* p);
	static void* GlobalAllocate(u64 size);
	static void GlobalFree(void* p);
private:
	void* pInternal;
};

}

#endif