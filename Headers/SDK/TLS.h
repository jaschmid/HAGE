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
#include <boost/thread.hpp>

namespace HAGE {

class TLS
{
public:
	static boost::thread_specific_ptr<guid> domain_guid;
	static boost::thread_specific_ptr<IDomain> domain_ptr;
	static boost::thread_specific_ptr<int> mode;
};

}

void* operator new (size_t size);
void* operator new[] (size_t size);
void operator delete (void *p);
void operator delete[] (void *p);

#endif