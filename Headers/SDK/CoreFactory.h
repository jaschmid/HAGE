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

template<u32 _Index> class _VoidInput;
typedef _VoidInput<0>	NoDirectInstantiation;

class Message;
class SharedCoreFactory;

class CoreFactory
{
public:
	CoreFactory(PinBase*& pOut,TaskManager* pTask);
	~CoreFactory();

	template<class _T> void SetDomain(_T* p)
	{
		pGuid = &guid_of<_T>::Get();
		pDomain = (IDomain*)domain_access<_T>::Get();
	}

	template<class _C> guid CreateObject(typename get_traits<_C>::ObjectInitType init)
	{
		return _CreateObject(guid_of<_C>::Get(),(void*)&init);
	}

	IObject* QueryObject(const guid& ObjectId);
	result DestroyObject(const guid& ObjectId);

	/*******************************/
	/* for each template functions */
	/*******************************/

	template<class _Result,class _ObjectType> u32 ForEachGetSome(const boost::function<bool (_ObjectType*,_Result&)>& f,_Result* pOut,u32 nOut,const guid& capability = guid_of<_ObjectType>::Get(),bool bSync = false)
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
	template<class _Result,class _ObjectType> u32 ForEachEx(const boost::function<_Result (_ObjectType*)>& f,_Result* pOut,u32 nOut,const guid& capability = guid_of<_ObjectType>::Get(),bool bSync = false)
	{
		static_assert( std::is_base_of<IObject,_ObjectType>::value ,"_T2 needs to have inherited from IObject!");
		const boost::function<bool (void*,IObject*)> c(
			[f] (void* v,IObject* o) -> bool  { (*reinterpret_cast<_Result*>(v)) = f(static_cast<_ObjectType*>(o)) ; return true;}
		);
		std::pair<const void*,u32> res= _ForEach(c,sizeof(_Result),capability,bSync,pOut,nOut,false);
		return res.second;
	}
	template<class _Result,class _ObjectType> std::pair<const _Result*,u32> ForEach(const boost::function<_Result (_ObjectType*)>& f,const guid& capability = guid_of<_ObjectType>::Get(),bool bSync = false)
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

	template<class _ObjectType,class _InitType> class InitFunction
	{
	public:
		typedef _ObjectType* (*func)(const guid&,const _InitType&);
		std::function<IObject* (const guid&,void*)> operator()()
		{
			const func fd=&_ObjectType::CreateInstance;
			return std::function<IObject* (const guid&,void*)>(
				[fd] (const guid& g,void* v) -> IObject* {return (IObject*)fd(g,*(const _InitType*)v);}
			);
		}
	};
	template<class _ObjectType,u32 i>   class InitFunction<_ObjectType,HAGE::_VoidInput<i>>
	{
	public:
		std::function<IObject* (const guid&,void*)> operator()()
		{
			return std::function<IObject* (const guid&,void*)>(
			);
		}
	};
	template<class _ObjectType>  class InitFunction<_ObjectType,void>
	{
	public:
		typedef _ObjectType* (*func)(const guid&);
		std::function<IObject* (const guid&,void*)> operator()()
		{
			const func fd=&_ObjectType::CreateInstance;
			return std::function<IObject* (const guid&,void*)>(
				[fd] (const guid& g,void* v) -> IObject* {return (IObject*)fd(g);}
			);
		}
	};
	template<class _ObjectType,class _InitType> class SubFunction
	{
	public:
		typedef _ObjectType* (*func)(const guid&,const MemHandle&,const guid&);

		std::function<IObject* (const guid&,void*)> operator()()
		{
			const func fd=&_ObjectType::CreateSub;
			static std::function<IObject* (const guid&,void*)> f = [fd] (const guid& g,void* v) -> IObject* {
					std::pair<const MemHandle&,const guid&>* pPair = (std::pair<const MemHandle&,const guid&>*)v;
					return (IObject*)(fd(g,pPair->first,pPair->second));
				};
			return f;
		}
	};
	template<class _ObjectType>  class SubFunction<_ObjectType,void>
	{
	public:
		std::function<IObject* (const guid&,void*)> operator()()
		{
			return std::function<IObject* (const guid&,void*)>(
			);
		}
	};
	template<class _ObjectType,u32 i>   class SubFunction<_ObjectType,_VoidInput<i>> : public SubFunction<_ObjectType,void>
	{
	};

	template<class _T> void RegisterObjectType()
	{
		InitFunction<_T,typename get_traits<_T>::ObjectInitType> InitGenerator;
		SubFunction<_T,typename get_traits<_T>::Input1Traits>	SubGenerator;
		std::function<IObject* (const guid&,void*)> init	= InitGenerator();
		std::function<IObject* (const guid&,void*)> sub		= SubGenerator();
		const guid& guidType  = guid_of<_T>::Get();
		const guid& guidInput1 = guid_of<typename _T::Input1::SourceClass>::Get();
		const guid& guidInput2 = guid_of<typename _T::Input2::SourceClass>::Get();
		const guid* guidCapabilities = &(_T::getCapabilities()[0]);
		u32			nCapabilities = (u32)_T::getCapabilities().size();

		_RegisterObjectType(((std::is_same<NoDirectInstantiation, typename get_traits<_T>::ObjectInitType>::value)?(nullptr):(&init)),&sub,guidType,guidInput1,guidInput2,guidCapabilities,nCapabilities);
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
			if( end > m_pFactory->m_ForEachTotalSize)
				end = m_pFactory->m_ForEachTotalSize;
			u32 element_size = m_pFactory->m_ForEachReturnSize;
			if(m_pFactory->m_ForEachGetSomeOut)
			{
				for(u32 i =  begin;i<end;++i)
				{
					if(m_pFactory->m_ForEachFunction(
						&m_pFactory->m_ForEachOut[i*element_size],
						m_pFactory->m_ForEachList->second[i].pRef->pObject
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
						m_pFactory->m_ForEachList->second[i].pRef->pObject
					);
				}
			}
		}

		CoreFactory*		m_pFactory;
		HAGE::u32			m_nMyIndex;
	};

	struct ObjectRefContainer;

	struct ObjectEntry
	{
		const guid*							objectId;
		ObjectRefContainer*					pRef;
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
		std::function<IObject* (const guid&,void*)>		init_function;
		bool											bNoInit;
		std::function<IObject* (const guid&,void*)>		sub_init;
		std::vector<Capability>							capabilities;
	};

	typedef std::unordered_map<guid,RegistrationContainer,guid_hasher>			function_map_type;

	struct ObjectRefContainer
	{
		IObject*		pObject;
		const function_map_type::value_type&		registration_entry;
		u32				uRefCounter;
		bool			bMaster;
		u64				nStepStamp;
		std::vector<u32> vCapabilitiesIndices;
		const void*		pResultMem[FRAME_BUFFER_COUNT];
		u32				nResultSize;
	};

	typedef std::unordered_map<guid,ObjectRefContainer,guid_hasher>				object_map_type;

	typedef std::unordered_map<guid,std::pair<guid,u32>,guid_hasher>			object_dependancy_map_type;

public:

	/***********************************/
	/* Indexing/Iterator Functionality */
	/***********************************/
#ifdef COMPILER_GCC
#define TYPENAME typename
#else
#define TYPENAME 
#endif

	class iterator : protected std::vector<ObjectEntry>::iterator
	{
		typedef TYPENAME std::vector<ObjectEntry>::iterator base;
#undef TYPENAME
		public:
			inline iterator operator ++()
			{
				return iterator(base::operator++(),m_pFactory);
			}
			inline iterator operator ++(int i)
			{
				return iterator(base::operator++(i),m_pFactory);
			}
			inline iterator operator --()
			{
				return iterator(base::operator--(),m_pFactory);
			}
			inline iterator operator --(int i)
			{
				return iterator(base::operator--(i),m_pFactory);
			}
			inline const guid& objectId() const
			{
				return *(base::operator*()).objectId;
			}
			inline const guid& classId() const
			{
				return (base::operator*()).pRef->registration_entry.first;
			}
			inline const IObject* object() const
			{
				return (base::operator*()).pRef->pObject;
			}
			inline const void* lastOut(u32 size) const
			{
				assert(isOutValid());
				assert(size == (base::operator*()).pRef->nResultSize);
				return (base::operator*()).pRef->pResultMem[m_pFactory->m_nReadIndex];
			}
			inline bool isOutValid() const
			{
				return ((base::operator*()).pRef->nStepStamp != m_pFactory->m_nCurrentStep);
			}
			inline const bool operator ==(const iterator& other) const
			{
				return (m_pFactory == other.m_pFactory) && ((base)*this) ==((base)other);
			}
			inline const bool operator !=(const iterator& other) const
			{
				return (m_pFactory != other.m_pFactory) || ((base)*this) !=((base)other);
			}
		private:
			iterator(const std::vector<ObjectEntry>::iterator& item,CoreFactory* pFactory) : std::vector<ObjectEntry>::iterator(item),m_pFactory(pFactory)
			{
			}

			CoreFactory* m_pFactory;
			friend class CoreFactory;
	};

	u32			size(const guid& capability = guidNull);
	iterator	begin(const guid& capability = guidNull);
	iterator	end(const guid& capability = guidNull);

private:

	// internal functions
	void	Step(u64 step){ if(m_pout) m_nReadIndex = m_pout->GetRead();m_nCurrentStep=step;}
	void	RegisterObjectOut( const guid& guid, const MemHandle& handle,u32 size);
	guid _CreateObject(const guid& ObjectTypeId, void* InitValue);
	std::pair<const void*,u32> _ForEach(boost::function<bool (void*,IObject*)> f,size_t size,const guid& capability,bool bSync,void* pDataOut,u32 nOut,bool bGetSome);
	void _RegisterObjectType(
			std::function<IObject* (const guid&,void*)>* fInit,
			std::function<IObject* (const guid&,void*)>* fSub,
			const guid& classType,const guid& guidInput1,const guid& guidInput2,const guid* guidCapabilities,u32 nCapabilities);
	void DestroyObjectInternal(object_map_type::iterator& item);
	bool TryCreateObjectWithGuid(const guid& ObjectId, const guid& ObjectTypeId, void* pInitData, bool bMaster = false);

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

	//for iterators
	u32 m_nReadIndex;//read index in the output pin
	u64 m_nCurrentStep;
	friend class SharedDomainBase;
	template<class _Domain> friend class DomainBase;
	template<class _ObjectOutputTraits> friend class _ObjectBaseOutput;

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
