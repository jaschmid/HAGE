#include <HAGE.h>

namespace HAGE
{
#ifdef COMPILER_MSVC
	__declspec(thread) TLS_data TLS::_data;
#else
	static void cleantlsempty(TLS_data* p){}

	boost::thread_specific_ptr<TLS_data> TLS::_data(&cleantlsempty);
#endif
}

void* operator new (size_t size)
{
	HAGE::TLS_data* tls = HAGE::TLS::getData();
	if(!tls->domain_ptr)
	{
		return HAGE::DomainMemory::GlobalAllocate(size);
	}
	void *p=tls->domain_ptr->Allocate(size); 
	if (p==0) // did malloc succeed?
	  throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}

void* operator new[] (size_t size)
{
	HAGE::TLS_data* tls = HAGE::TLS::getData();
	if(!tls->domain_ptr)
	{
		return HAGE::DomainMemory::GlobalAllocate(size);
	}
	void *p=tls->domain_ptr->Allocate(size); 
	if (p==0) // did malloc succeed?
	  throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}

void operator delete (void *p)
{
	HAGE::TLS_data* tls = HAGE::TLS::getData();
	if(!tls->domain_ptr)
	{
		HAGE::DomainMemory::GlobalFree(p);
		return;
	}
	tls->domain_ptr->Free(p); 
}

void operator delete[] (void *p)
{
	HAGE::TLS_data* tls = HAGE::TLS::getData();
	if(!(tls->domain_ptr))
	{
		HAGE::DomainMemory::GlobalFree(p);
		return;
	}
	tls->domain_ptr->Free(p); 
}