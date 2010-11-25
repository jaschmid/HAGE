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

#ifdef COMPILER_GCC
#include <tr1/type_traits>
#endif

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
		pGuid = &guid_of<_T>::value;
		pDomain = (IDomain*)domain_access<_T>::Get();
	}

	guid CreateObject(const guid& ObjectTypeId, IObject** ppInterface = nullptr);
	IObject* QueryObject(const guid& ObjectId);
	result DestroyObject(const guid& ObjectId);

	u32	GetNumObjects(const guid& capability = guidNull);
	template<class _Result,class _ObjectType> u32 ForEachGetSome(const boost::function<bool (_ObjectType*,_Result&)>& f,_Result* pOut,u32 nOut,const guid& capability = guidNull,bool bSync = false)
	{
		static_assert( std::is_base_of<IObject,_ObjectType>::value ,"_T2 needs to have inherited from IObject!");
		if(std::tr1::has_trivial_constructor<_Result>::value)
		{
			const boost::function<bool (void*,IObject*)> c(
				[f] (void* v,IObject* o) -> bool { return f(static_cast<_ObjectType*>(o),*reinterpret_cast<_Result*>(v)) ;}
			);
			std::pair<const void*,u32> res= _ForEach(c,sizeof(_Result),capability,bSync,pOut,nOut,true);
			return res.second;
		}
		else
		{
			const boost::function<bool (void*,IObject*)> c(
				[f] (void* v,IObject* o) -> bool { new(v)_Result; return f(static_cast<_ObjectType*>(o),*reinterpret_cast<_Result*>(v)) ;}
			);
			std::pair<const void*,u32> res= _ForEach(c,sizeof(_Result),capability,bSync,pOut,nOut,true);
			return res.second;
		}
	}
	template<class _Result,class _ObjectType> u32 ForEachEx(const boost::function<_Result (_ObjectType*)>& f,_Result* pOut,u32 nOut,const guid& capability = guidNull,bool bSync = false)
	{
		static_assert( std::is_base_of<IObject,_ObjectType>::value ,"_T2 needs to have inherited from IObject!");
		const boost::function<bool (void*,IObject*)> c(
			[f] (void* v,IObject* o) -> bool  { (*reinterpret_cast<_Result*>(v)) = f(static_cast<_ObjectType*>(o)) ; return true;}
		);
		std::pair<const void*,u32> res= _ForEach(c,sizeof(_Result),capability,bSync,pOut,nOut,false);
		return res.second;
	}
	template<class _Result,class _ObjectType> std::pair<const _Result*,u32> ForEach(const boost::function<_Result (_ObjectType*)>& f,const guid& capability = guidNull,bool bSync = false)
	{
		static_assert( std::has_trivial_destructor<_Result>::value ,"_T needs to have trivial destructor, use ForEachEx to perform custom destruction of results!");
		static_assert( std::is_base_of<IObject,_ObjectType>::value ,"_T2 needs to have inherited from IObject!");
		const boost::function<bool (void*,IObject*)> c(
			[f] (void* v,IObject* o) -> bool  { (*reinterpret_cast<_Result*>(v)) = f(static_cast<_ObjectType*>(o)) ; return true;}
		);
		std::pair<const void*,u32> res= _ForEach(c,sizeof(_Result),capability,bSync,nullptr,0,false);
		return std::pair<const _Result*,u32>(reinterpret_cast<const _Result*>(res.first),res.second);
	}

	bool MessageProc(const MessageFactoryUnknown* pMessage);
	bool DispatchMessage(const MessageObjectUnknown* pMessage);
	void Shutdown();

	template<class _T> void RegisterObjectType()
	{
		if(_T::isImplemented)
		{
			RegistrationContainer r ={&_T::CreateInstance, std::vector<Capability>()};
			const guid& classguid=guid_of<_T>::value;
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

			const guid& guid1 = guid_of<typename _T::Input1::SourceClass>::value;

			if(guid1 != guidNull)
			{
				assert(objectDependancyMap.find(guid1) == objectDependancyMap.end());
				objectDependancyMap.insert(object_dependancy_map_type::value_type(guid1,guid_of<_T>::value));

				const guid& guid2 = guid_of<typename _T::Input2::SourceClass>::value;
				if(guid2 != guidNull)
				{
					assert(objectDependancyMap.find(guid2) == objectDependancyMap.end());
					objectDependancyMap.insert(object_dependancy_map_type::value_type(guid2,guid_of<_T>::value));
				}
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

		virtual void operator() ()
		{
			u32 begin =  m_nMyIndex*m_pFactory->m_ForEachGroupSize;
			u32 end = begin+m_pFactory->m_ForEachGroupSize;
			if( end >= m_pFactory->m_ForEachTotalSize)
				end = m_pFactory->m_ForEachTotalSize-1;
			u32 element_size = m_pFactory->m_ForEachReturnSize;
			if(m_pFactory->m_ForEachGetSomeOut)
			{
				for(u32 i =  begin;i<end;++i)
				{
					if(m_pFactory->m_ForEachFunction(
						&m_pFactory->m_ForEachOut[i*element_size],
						m_pFactory->m_ForEachList->second[i].pObject
					))
					{
						u32 nOutIndex = _InterlockedIncrement((i32*)&m_pFactory->m_ForEachGetSomeCounter);
						memcpy(&m_pFactory->m_ForEachGetSomeOut[(nOutIndex-1)*element_size],&m_pFactory->m_ForEachOut[i*element_size],element_size);
					}
				}
			}
			else
			{
				for(u32 i =  begin;i<end;++i)
				{
					m_pFactory->m_ForEachFunction(
						&m_pFactory->m_ForEachOut[i*element_size],
						m_pFactory->m_ForEachList->second[i].pObject
					);
				}
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

	typedef std::unordered_map<guid,guid,guid_hasher>							object_dependancy_map_type;

	std::pair<const void*,u32> _ForEach(boost::function<bool (void*,IObject*)> f,size_t size,const guid& capability,bool bSync,void* pDataOut,u32 nOut,bool bGetSome);
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
	object_dependancy_map_type	objectDependancyMap;

	//for foreach
	std::vector<ForEachTask>					m_ForEachTasks;
	std::vector<u8>								m_ForEachReturnBuffer;
	u32											m_ForEachReturnSize;
	u32											m_ForEachGroupSize;
	u32											m_ForEachTotalSize;
	u8*											m_ForEachOut;
	u8*											m_ForEachGetSomeOut;
	volatile u32								m_ForEachGetSomeCounter;
	object_capabilties_map_type::value_type*	m_ForEachList;
	boost::function<bool (void*,IObject*)>		m_ForEachFunction;
};

}

#endif
