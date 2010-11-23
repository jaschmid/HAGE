/********************************************************/
/* FILE: CoreFactory.h                                  */
/* DESCRIPTION: Defines the CoreFactory class for       */
/*              Domains                                 */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __COREFACTORY_H__
#define __COREFACTORY_H__

#include "HAGE.h"
#include "PinBase.h"
#include <boost/function.hpp>

#include <map>

#include <boost/random/mersenne_twister.hpp>
#include <boost/uuid/random_generator.hpp>
#include <unordered_map>
#include <functional>
#include <type_traits>

namespace HAGE {

class Message;
class SharedCoreFactory;

class CoreFactory
{
public:
	CoreFactory(PinBase*& pOut,TaskManager* pTask);
	~CoreFactory();

	template<class _T> void SetDomain(_T* p)
	{
		pGuid = &_T::id;
		pDomain = (IDomain*)_T::pDomain;
	}

	guid CreateObject(const guid& ObjectTypeId, IObject** ppInterface);
	result QueryObject(const guid& ObjectId, IObject** ppInterface);
	result DestroyObject(const guid& ObjectId);

	template<class _T,class _T2> std::pair<const _T*,u32> ForEach(const boost::function<_T (_T2*)>& f,const guid& capability,bool bSync = false)
	{/*
		static_assert( std::has_trivial_constructor<_T>::value ,"_T needs to have trivial constructor!");
		static_assert( std::has_trivial_destructor<_T>::value ,"_T needs to have trivial deconstructor!");*/
		const boost::function<void (void*,IObject*)> c(
			[f] (void* v,IObject* o) { (*reinterpret_cast<_T*>(v)) = f(static_cast<_T2*>(o)) ;}
		);
		std::pair<const void*,u32> res= _ForEach(c,sizeof(_T),capability,bSync);
		return std::pair<const _T*,u32>(reinterpret_cast<const _T*>(res.first),res.second);
	}
	bool MessageProc(const MessageFactoryUnknown* pMessage);
	bool DispatchMessage(const MessageObjectUnknown* pMessage);
	void Shutdown();

	template<class _T> void RegisterObjectType()
	{
		if(_T::isImplemented)
		{
			RegistrationContainer r ={&_T::CreateInstance, std::vector<Capability>()};
			const guid& classguid=_T::getClassGuid();
			registeredFunctionCreation.insert(
				function_map_type::value_type(classguid,r)
			);
			function_map_type::iterator inserted = registeredFunctionCreation.find(classguid);
			assert(inserted != registeredFunctionCreation.end());
			inserted->second.capabilities.reserve(_T::getCapabilities().size());
			for(auto it=_T::getCapabilities().begin();it!=_T::getCapabilities().end();++it)
			{
				auto found = capabilitiesObjectLists.find(*it);
				if(found ==capabilitiesObjectLists.end())
				{
					capabilitiesObjectLists.insert(object_capabilties_map_type::value_type(*it,std::vector<ObjectEntry>()));
				}
				found = capabilitiesObjectLists.find(*it);
				Capability c = {*it,&(*found)};
				inserted->second.capabilities.push_back( c );
			}
		}
	}
private:

	struct guid_hasher
	{
		size_t operator()(const guid& g) const
		{
			return (size_t)(g.ll[0] ^ g.ll[1]);
		}
	};

	struct ForEachTask : public TaskManager::genericTask
	{

		void operator() ()
		{
			u32 begin =  m_nMyIndex*m_pFactory->m_ForEachGroupSize;
			u32 end = begin+m_pFactory->m_ForEachGroupSize;
			if( end >= m_pFactory->m_ForEachTotalSize)
				end = m_pFactory->m_ForEachTotalSize-1;
			for(u32 i =  begin;i<end;++i)
			{
				m_pFactory->m_ForEachFunction(
					&m_pFactory->m_ForEachReturnBuffer[i*m_pFactory->m_ForEachReturnSize],
					m_pFactory->m_ForEachList->second[i].pObject
				);
			}
		}

		CoreFactory*		m_pFactory;
		HAGE::u32			m_nMyIndex;
	};

	struct ObjectEntry
	{
		IObject*							pObject;
		std::vector<u32>*		            object_entry;
		u32									index;
	};

	typedef std::unordered_map<guid,std::vector<ObjectEntry>,guid_hasher>		object_capabilties_map_type;

	struct Capability
	{
		guid										cap_guid;
		object_capabilties_map_type::value_type*	pEntry;
	};

	struct RegistrationContainer
	{
		std::function<IObject*(guid)>	function;
		std::vector<Capability>			capabilities;
	};

	typedef std::unordered_map<guid,RegistrationContainer,guid_hasher>			function_map_type;

	struct ObjectRefContainer
	{
		IObject*		pObject;
		const function_map_type::value_type&		registration_entry;
		u32				uRefCounter;
		bool			bMaster;
		std::vector<u32> vCapabilitiesIndices;
	};

	typedef std::unordered_map<guid,ObjectRefContainer,guid_hasher>				object_map_type;


	std::pair<const void*,u32> _ForEach(boost::function<void (void*,IObject*)> f,size_t size,const guid& capability,bool bSync);
	void DestroyObjectInternal(object_map_type::iterator& item);

	bool TryCreateObjectWithGuid(const guid& ObjectId, const guid& ObjectTypeId, IObject** ppInterface, bool bMaster = false);

	static SharedCoreFactory*	pCoreFactory;

	PinBase*&					m_pout;
	IDomain*					pDomain;
	const guid*					pGuid;

	TaskManager*				m_pTask;

	boost::uuids::basic_random_generator<boost::mt19937> number_generator;

	function_map_type			registeredFunctionCreation;
	object_map_type				flatObjectList;
	object_capabilties_map_type capabilitiesObjectLists;

	//for foreach
	std::vector<ForEachTask>					m_ForEachTasks;
	std::vector<u8>								m_ForEachReturnBuffer;
	u32											m_ForEachReturnSize;
	u32											m_ForEachGroupSize;
	u32											m_ForEachTotalSize;
	object_capabilties_map_type::value_type*	m_ForEachList;
	boost::function<void (void*,IObject*)>		m_ForEachFunction;
};

}

#endif
