#include <HAGE.h>

#ifdef TARGET_WINDOWS

#include <Windows.h>

namespace HAGE {

DomainMemory::DomainMemory() :pInternal(HeapCreate(HEAP_NO_SERIALIZE | HEAP_GENERATE_EXCEPTIONS, 0, 0))
{
}

DomainMemory::~DomainMemory()
{
	HeapDestroy(pInternal);
}

void* DomainMemory::Allocate(u64 size)
{
	return HeapAlloc(pInternal,0,(SIZE_T)size);
}

void DomainMemory::Free(void* p)
{
	HeapFree(pInternal,0,p);
}

void* DomainMemory::GlobalAllocate(u64 size)
{
	return HeapAlloc(GetProcessHeap(),0,(SIZE_T)size);
}

void DomainMemory::GlobalFree(void* p)
{
	HeapFree(GetProcessHeap(),0,p);
}

}

#else

namespace HAGE {

DomainMemory::DomainMemory() :pInternal(nullptr)
{
}

DomainMemory::~DomainMemory()
{
}

void* DomainMemory::Allocate(u64 size)
{
	return malloc((size_t)size);
}

void DomainMemory::Free(void* p)
{
	free(p);
}

void* DomainMemory::GlobalAllocate(u64 size)
{
	return malloc((size_t)size);
}

void DomainMemory::GlobalFree(void* p)
{
	free(p);
}

}

#endif
