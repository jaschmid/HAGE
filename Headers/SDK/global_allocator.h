#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef GLOBAL_ALLOCATOR_INCLUDED_H
#define GLOBAL_ALLOCATOR_INCLUDED_H

#include "DomainMemory.h"

namespace HAGE {

template <class T> class global_allocator
{
public:
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;

  global_allocator() {}
  global_allocator(const global_allocator&) {}



  pointer   allocate(size_type n, const void * = 0) 
  {
	T* t = (T*) DomainMemory::GlobalAllocate(n * sizeof(T));
    return t;
  }
  
  void      deallocate(void* p, size_type) {
              if (p) {
                DomainMemory::GlobalFree(p);
              } 
            }

  pointer           address(reference x) const { return &x; }
  const_pointer     address(const_reference x) const { return &x; }
  global_allocator<T>&  operator=(const global_allocator&) { return *this; }
  void              construct(pointer p, const T& val) 
                    { new ((T*) p) T(val); }
  void              destroy(pointer p) { p->~T(); }

  size_type         max_size() const { return size_t(-1); }

  template <class U>
  struct rebind { typedef global_allocator<U> other; };

  template <class U>
  global_allocator(const global_allocator<U>&) {}

  template <class U>
  global_allocator& operator=(const global_allocator<U>&) { return *this; }
};

}

#endif