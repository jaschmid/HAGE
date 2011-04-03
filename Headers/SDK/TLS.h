/********************************************************/
/* FILE: TLS.h                                          */
/* DESCRIPTION: Declares thread local storage for HAGE. */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef TLS_H_INCLUDED__
#define TLS_H_INCLUDED__

#include "types.h"
#include "IDomain.h"
#include "DomainMemory.h"
#include <boost/thread.hpp>

namespace HAGE {

struct TLS_data
{
	guid				domain_guid;
	IDomain*			domain_ptr;
	i32					mode;
	u32					thread_id;
	IRandomSource*		random_generator;
};

class TLS
{
public: 

#ifdef COMPILER_MSVC
	static void Init(u32 thread_id)
	{
		_data.domain_guid = guidNull;
		_data.domain_ptr = nullptr;
		_data.random_generator = nullptr;
		_data.mode = 0;
		_data.thread_id = thread_id;
	}

	static TLS_data* getData()
	{
		return &_data;
	}

	static void Free()
	{
		//nothing to do
	}


#else
	static void Init(u32 thread_id)
	{
		TLS_data* pData=(TLS_data*)DomainMemory::GlobalAlloc(sizeof(TLS_data));
		pData->domain_guid = guidNull;
		pData->domain_ptr = nullptr;
		pData->random_generator = nullptr;
		pData->mode = 0;
		pData->thread_id = thread_id;
		_data.reset(pData);
	}

	static TLS_data* getData()
	{
		return (TLS_Data*)_data;
	}

	static void Free()
	{
		DomainMemory::GlobalFree((TLS_Data*)_data);
		_data.release();
	}
#endif

public:

#ifdef COMPILER_MSVC
	__declspec(thread) static TLS_data _data;
#else
	static boost::thread_specific_ptr<TLS_data> _data;
#endif
};

}

void* operator new (size_t size);
void* operator new[] (size_t size);
void operator delete (void *p);
void operator delete[] (void *p);

#endif