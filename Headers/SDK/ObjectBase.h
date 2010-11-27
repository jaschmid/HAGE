/********************************************************/
/* FILE: ObjectBase.h                                   */
/* DESCRIPTION: Defines ObjectBase class                */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef OBJECT__BASE__INCLUDED
#define OBJECT__BASE__INCLUDED

#include "HAGE.h"
#include "TaskManager.h"
#include "PinHelpers.h"
#include <map>

namespace HAGE {

template<class _Domain> class DomainMember
{
protected:

	typedef	_Domain Domain;

	static inline PinBase& GetOutputPin()
	{
		return OutputPin<Domain>::pin;
	}

	static inline void GenerateShutdownMessage()
	{
	    domain_access<_Domain>::Get()->GenerateShutdownMessage();
	}

	static inline void PostMessage(const Message& message)
	{
		domain_access<_Domain>::Get()->PostMessage(message);
	}

	static inline CoreFactory* GetFactory()
	{
		return &(domain_access<_Domain>::Get()->Factory);
	}

	static inline TaskManager* GetTasks()
	{
		return &(domain_access<_Domain>::Get()->Tasks);
	}

	template<class _SourceDomain> static inline PinBase& GetInputPin()
	{
		return InputPin<_SourceDomain,Domain>::pin;
	}
};


template<class _Domain> class _ObjectBaseDomain : public DomainMember<_Domain>, public IObject
{
protected:

	_ObjectBaseDomain(const guid& guidObjectId) : m_guidObjectId(guidObjectId)
	{

	}

	const guid								m_guidObjectId;

private:

	virtual result Destroy()
	{
		delete this;
		return S_OK;
	}

	template<class _D> friend class _ObjectBaseOutput;
	template<class _D> friend class _ObjectBaseInput;
};

template<class _ObjectInputTraits> class _ObjectBaseInput
{
protected:
	typedef _ObjectInputTraits										Traits;
	typedef typename _ObjectInputTraits::Domain						Domain;
	typedef typename _ObjectInputTraits::SourceClass				SourceClass;
	typedef typename get_traits<SourceClass>::OutputTraits::Domain		SourceDomain;
	typedef typename get_traits<SourceClass>::OutputTraits::Type			Type;

	_ObjectBaseInput() : pin(typename typename get_traits<SourceDomain>::DomainBaseType::Output::GetBasePin())
	{
	}
	_ObjectBaseInput(const MemHandle& h,const guid& source) : pin(typename typename get_traits<SourceDomain>::DomainBaseType::Output::GetBasePin())
	{
		if(source == guid_of<SourceDomain>::Get())
		{
			handle = h;
			pin.ReferenceMemBlock(handle);
		}
	}
	~_ObjectBaseInput()
	{
		if(handle.isValid())
		{
			pin.FreeMemBlock(handle);
			handle = MemHandle();
		}
	}
	inline bool IsReady() const
	{
		return handle.isValid();
	}
	inline const Type& Get() const
	{
		assert(handle.isValid());
		return *(const Type*)pin.GetReadMem(handle,sizeof(Type));
	}

private:

	inline void Open(MemHandle h)
	{
		handle = h;
		pin.ReferenceMemBlock(handle);
	}

	PinBase& pin;
	MemHandle handle;

	friend class CoreFactory;
	template<class _TraintsX> friend class ObjectBase;
};

template<u32 i> class _ObjectBaseInput< _VoidInput<i> >
{
protected:
	_ObjectBaseInput()
	{
	}
	_ObjectBaseInput(const MemHandle& h,const guid& source)
	{
	}
	//empty
	typedef VoidTraits							Traits;
	typedef void	Domain;
	typedef void	SourceDomain;
	typedef void	SourceClass;
	typedef void	Type;
private:

	inline void Open(MemHandle h)
	{
		assert("Attempted to call void Object Input Open");
	}

	friend class CoreFactory;
	template<class _FinalX> friend class ObjectBase;
};

template<class _ObjectOutputTraits> class _ObjectBaseOutput
{
protected:
	typedef _ObjectOutputTraits				Traits;
	typedef typename _ObjectOutputTraits::Type		Type;
	typedef typename _ObjectOutputTraits::Domain		Domain;

	_ObjectBaseOutput(const guid& objectId) : pin(typename get_traits<Domain>::DomainBaseType::Output::GetBasePin())
	{
		__out_handle=pin.AllocateMemBlock(sizeof(Type));
		assert(__out_handle.isValid());
		domain_access<Domain>::Get()->Factory.RegisterObjectOut(objectId,__out_handle,sizeof(Type));
	}
	~_ObjectBaseOutput()
	{
		if(__out_handle.isValid())
		{
			pin.FreeMemBlock(__out_handle);
			__out_handle = MemHandle();
		}
	}
	inline void Open(MemHandle h)
	{
		pin.ReferenceMemBlock(h);
		__out_handle = h;
	}
	inline bool IsReady() const
	{
		return __out_handle.isValid();
	}
	inline const Type& GetOld() const
	{
		assert(__out_handle.isValid());
		return *(const Type*)pin.GetReadMem(__out_handle,sizeof(Type));
	}
	inline void Set(const Type& value)
	{
		assert(__out_handle.isValid());
		*(Type*)pin.GetWriteMem(__out_handle,sizeof(Type)) = value;
	}
	inline Type* Access()
	{
		assert(__out_handle.isValid());
		return (Type*)pin.GetWriteMem(__out_handle,sizeof(Type));
	}

private:
	PinBase& pin;
	MemHandle __out_handle;

	template<class _DomainX> friend class _ObjectBaseInput;
	friend class CoreFactory;
};

template<> class _ObjectBaseOutput<VoidTraits>
{
protected:
	typedef VoidTraits	Traits;
	typedef void		Type;
	typedef void		Domain;

	//empty
	_ObjectBaseOutput(const guid& objectId)
	{
	}

	template<class _DomainX> friend class _ObjectBaseInput;
};


template<class _Final> class ObjectBase :
	public	_ObjectBaseOutput<typename get_traits<_Final>::OutputTraits>,
	public	_ObjectBaseInput<typename get_traits<_Final>::Input1Traits>,
	public	_ObjectBaseInput<typename get_traits<_Final>::Input2Traits>,
	public	_ObjectBaseDomain<typename get_traits<_Final>::Domain>
{
protected:
	typedef typename get_traits<_Final>::Domain					Domain;
	typedef	_Final												Final;
	typedef get_traits<_Final>									Traits;
	typedef _ObjectBaseOutput<typename Traits::OutputTraits>	Output;
	typedef _ObjectBaseInput<typename Traits::Input1Traits>		Input1;
	typedef _ObjectBaseInput<typename Traits::Input2Traits>		Input2;
	typedef ObjectBase<_Final>									ObjectBaseType;
public:
	static const std::array<guid,2>& getCapabilities(){static const std::array<guid,2> val= {guidNull,guid_of<Final>::Get()}; return val;}
protected:
	ObjectBase(const guid& rguid) : _ObjectBaseDomain<typename get_traits<_Final>::Domain>(rguid),_ObjectBaseOutput<typename Traits::OutputTraits>(rguid) {}
	ObjectBase(const guid& rguid,const MemHandle& h,const guid& source) 
		:	_ObjectBaseDomain<typename get_traits<_Final>::Domain>(rguid),
			_ObjectBaseOutput<typename Traits::OutputTraits>(rguid),
			_ObjectBaseInput<typename Traits::Input1Traits>(h,source),
			_ObjectBaseInput<typename Traits::Input2Traits>(h,source) {}
	virtual ~ObjectBase(){}

private:
	virtual bool MessageProc(const MessageObjectUnknown* pMessage)
	{
		if(pMessage->GetMessageCode() == MESSAGE_OBJECT_OUTPUT_INIT)
		{
			MessageObjectOutputInit* pDetailed = (MessageObjectOutputInit*)pMessage;
			if(pDetailed->GetSource() == guid_of<get_traits<typename Input1::SourceDomain>>::Get() )
				Input1::Open(pDetailed->GetHandle());
			else if(pDetailed->GetSource() == guid_of<get_traits<typename Input2::SourceDomain>>::Get() )
				Input2::Open(pDetailed->GetHandle());
			else
				assert("Recieved input from source, but no handler to recieve it!\n");
			return true;
		}
		return false;
	}

	friend class CoreFactory;
	template<class _DomainX> friend  class _ObjectBaseInput;
	template<class _FinalX> friend class ObjectBase;
};

template<u32 index> class guid_of<_VoidInput<index>> : public guid_of<void>
{
};

}

#endif
