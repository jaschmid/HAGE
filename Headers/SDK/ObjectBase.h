#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef OBJECT__BASE__INCLUDED
#define OBJECT__BASE__INCLUDED

#include "HAGE.h"
#include "TaskManager.h"
#include "PinHelpers.h"

namespace HAGE {

template<class _T> class ObjectBase
{
protected:
	static _T*&						pDomain;
	static const guid&				guidDomain;

	static PinBase& GetOutputPin()
	{
		return OutputPin<_T>::pin;
	}

	static void GenerateShutdownMessage()
	{
	    pDomain->GenerateShutdownMessage();
	}

	static void PostMessage(const Message& message)
	{
		pDomain->PostMessage(message);
	}

	template<class _T2> static PinBase& GetInputPin()
	{
		return InputPin<_T2,_T>::pin;
	}
};

template<class _T> _T*& ObjectBase<_T>::pDomain = (_T*&)_T::pDomain;
template<class _T> const guid& ObjectBase<_T>::guidDomain = _T::id;

}

#endif
