#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __COREFACTORY_H__
#define __COREFACTORY_H__

#include "HAGE.h"
#include "PinBase.h"

#include <map>

#include <boost/random/mersenne_twister.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/function.hpp>

namespace HAGE {

class Message;
class SharedCoreFactory;

class CoreFactory
{
public:
	CoreFactory(PinBase*& pOut);
	~CoreFactory();

	template<class _T> void SetDomain(_T* p)
	{
		pGuid = &_T::id;
		pDomain = (IDomain*)_T::pDomain;
	}

	guid CreateObject(const guid& ObjectTypeId, IObject** ppInterface);
	result QueryObject(const guid& ObjectId, IObject** ppInterface);
	result DestroyObject(const guid& ObjectId);
	bool MessageProc(const MessageFactoryUnknown* pMessage);
	bool DispatchMessage(const MessageObjectUnknown* pMessage);
	void Shutdown();

	template<class _T> void RegisterObjectType()
	{
		if(_T::isImplemented)
		{
			registeredFunctionCreation.insert(std::pair<guid,boost::function<IObject*(guid)>>(_T::getClassGuid(),&_T::CreateInstance));
		}
	}
private:

	struct ObjectRefContainer
	{
		IObject*		pObject;
		u32				uRefCounter;
		bool			bMaster;
	};

	bool TryCreateObjectWithGuid(const guid& ObjectId, const guid& ObjectTypeId, IObject** ppInterface, bool bMaster = false);

	static SharedCoreFactory*	pCoreFactory;

	PinBase*&					m_pout;
	IDomain*					pDomain;
	const guid*					pGuid;

	boost::uuids::basic_random_generator<boost::mt19937> number_generator;

	typedef std::map<guid,boost::function<IObject*(guid)>> function_map_type;
	typedef std::map<guid,ObjectRefContainer> object_map_type;

	function_map_type			registeredFunctionCreation;
	object_map_type				flatObjectList;
};

}

#endif
