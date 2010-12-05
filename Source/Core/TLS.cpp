#include <HAGE.h>

namespace HAGE
{
	
	static void cleanguidempty(guid* p){}
	static void cleandomainempty(IDomain* p){}
	static void cleanthreadmodeempty(int* p){}
	static void cleanthreadidempty(u32* p){}

	boost::thread_specific_ptr<guid> TLS::domain_guid(&cleanguidempty);
	boost::thread_specific_ptr<IDomain> TLS::domain_ptr(&cleandomainempty);
	boost::thread_specific_ptr<int> TLS::mode(&cleanthreadmodeempty);
	boost::thread_specific_ptr<u32> TLS::thread_id(&cleanthreadidempty);
}

void* operator new (size_t size)
{
	if(!HAGE::TLS::domain_ptr.get())
	{
		return HAGE::DomainMemory::GlobalAllocate(size);
	}
	void *p=HAGE::TLS::domain_ptr->Allocate(size); 
	if (p==0) // did malloc succeed?
	  throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}

void* operator new[] (size_t size)
{
	if(!HAGE::TLS::domain_ptr.get())
	{
		return HAGE::DomainMemory::GlobalAllocate(size);
	}
	void *p=HAGE::TLS::domain_ptr->Allocate(size); 
	if (p==0) // did malloc succeed?
	  throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}

void operator delete (void *p)
{
	if(!HAGE::TLS::domain_ptr.get())
	{
		HAGE::DomainMemory::GlobalFree(p);
		return;
	}
	HAGE::TLS::domain_ptr->Free(p); 
}

void operator delete[] (void *p)
{
	if(!HAGE::TLS::domain_ptr.get())
	{
		HAGE::DomainMemory::GlobalFree(p);
		return;
	}
	HAGE::TLS::domain_ptr->Free(p); 
}