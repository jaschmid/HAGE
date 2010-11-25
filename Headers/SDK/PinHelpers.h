/********************************************************/
/* FILE: PinHelpers.h                                   */
/* DESCRIPTION: Defines helper classes for PinBase      */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef PIN_HELPERS_H_INCLUDED
#define PIN_HELPERS_H_INCLUDED

#include "PinBase.h"
#include "DomainBase.h"

namespace HAGE {
	
template<class _T,class _T2> class InputPin;

template<class _T> class OutputPin
{
public:
	OutputPin()
	{
		((SharedDomainBase*)DomainBase<_T>::pDomain)->RegisterOutput(pin);
	}
	template<class _C> inline _C* GetMem(MemHandle p)
	{
		return (_C*)pin.GetWriteMem(p,sizeof(_C));
	}
	template<class _C> inline MemHandle Alloc()
	{
		return pin.AllocateMemBlock(sizeof(_C));
	}
	inline void Free(MemHandle p)
	{
		return pin.FreeMemBlock(p);
	}
	template<class _TX> inline void PostMessage(const _TX& message)
	{
		pin.PostMessage((const Message&)message);
	}
	inline PinBase& GetBasePin()
	{
		return pin;
	}

private:
	static PinBase pin;
	template<class _1,class _2> friend class InputPin;
	template <class _1> friend class OutputVar;
	template <class _1> friend class InputVar;
	template<class _1> friend class ObjectBase;
};

template<class _T> PinBase OutputPin<_T>::pin;

template<class _T,class _T2> class InputPin
{
public:
	InputPin(i32 delay = 0)
	{
		((SharedDomainBase*)DomainBase<_T2>::pDomain)->RegisterInput(pin,delay);
	}
	template<class _C> inline const _C* GetMem(MemHandle p)
	{
		return (const _C*)pin.GetReadMem(p,sizeof(_C));
	}
	inline void Free(MemHandle p)
	{
		return pin.FreeMemBlock(p);
	}
	inline const Message* GetNextMessage(const Message* message)
	{
		return pin.GetNextMessage(message);
	}
	inline PinBase& GetBasePin()
	{
		return pin;
	}

private:
	static PinBase& pin;
	template<class _1> friend class InputVar;
	template<class _1> friend class ObjectBase;
};

template<class _T,class _T2> PinBase& InputPin<_T,_T2>::pin = OutputPin<_T>::pin;

template <class _T> class InputVar
{
public:
	InputVar(PinBase& Pin) : pin(Pin)
	{
	}
	~InputVar()
	{
		if(handle.isValid())
		{
			pin.FreeMemBlock(handle);
			handle = MemHandle();
		}
	}
	void Open(MemHandle h)
	{
		pin.ReferenceMemBlock(h);
		handle = h;
	}
	bool IsOpen()
	{
		return handle.isValid();
	}
	const _T* operator->()
	{
		assert(handle.isValid());
		return (const _T*)pin.GetReadMem(handle,sizeof(_T));
	}
	const _T& operator*()
	{
		assert(handle.isValid());
		return *(const _T*)pin.GetReadMem(handle,sizeof(_T));
	}
private:
	PinBase& pin;
	MemHandle handle;
};

template <class _T> class OutputVar
{
public:
	OutputVar(PinBase& Pin) : pin(Pin)
	{
		handle=pin.AllocateMemBlock(sizeof(_T));
	}
	~OutputVar()
	{
		if(handle.isValid())
		{
			pin.FreeMemBlock(handle);
			handle = MemHandle();
		}
	}
	_T* operator->()
	{
		assert(handle.isValid());
		return (_T*)pin.GetWriteMem(handle,sizeof(_T));
	}
	_T& operator*()
	{
		assert(handle.isValid());
		return *(_T*)pin.GetWriteMem(handle,sizeof(_T));
	}
	MemHandle GetHandle() {return handle;}
private:
	PinBase& pin;
	MemHandle handle;
};


template<class _T,class _T2> class InputMessages;

template<class _T> class OutputMessages
{
public:
	OutputMessages()
	{
		((SharedDomainBase*)DomainBase<_T>::pDomain)->RegisterOutput(messageQueue);
	}

private:
	static LockedMessageQueue messageQueue;
	template<class _1,class _2> friend class InputMessages;
};

template<class _T> LockedMessageQueue OutputMessages<_T>::messageQueue;

template<class _T,class _T2> class InputMessages
{
public:
	InputMessages(u32 delay=0) : messageQueue(OutputMessages<_T>::messageQueue)
	{
		((SharedDomainBase*)DomainBase<_T2>::pDomain)->RegisterInput(messageQueue,delay);
	}

private:
	LockedMessageQueue& messageQueue;
};

}

#endif