/********************************************************/
/* FILE: IDomain.h                                      */
/* DESCRIPTION: Defines IDomain interface               */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __IDOMAIN_H__
#define __IDOMAIN_H__

namespace HAGE {

class IDomain
{
public:
	virtual void* Allocate(u64 size) = 0;
	virtual void Free(void* p) = 0;
};

template<class _Domain> class domain_access
{
};

}

#endif
