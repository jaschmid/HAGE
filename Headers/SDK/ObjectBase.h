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

	static PinBase& GetOutputPin()
	{
		return OutputPin<Domain>::pin;
	}

	static void GenerateShutdownMessage()
	{
	    domain_access<_Domain>::Get()->GenerateShutdownMessage();
	}

	static void PostMessage(const Message& message)
	{
		domain_access<_Domain>::Get()->PostMessage(message);
	}

	template<class _SourceDomain> static PinBase& GetInputPin()
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

	template<class _D,class _OutputType> friend class _ObjectBaseOutput;
	template<class _D,class _Input> friend class _ObjectBaseInput;
};

template<class _Domain,class _Input> class _ObjectBaseInput
{
protected:
	typedef _Domain				    Domain;
	typedef _Input					SourceClass;
	typedef typename _Input::Output::Domain	Source;
	typedef typename _Input::Output::Type	Type;

	_ObjectBaseInput() : pin(InputPin<Source,Domain>::pin)
	{
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
		pin.ReferenceMemBlock(h);
		handle = h;
	}

	PinBase& pin;
	MemHandle handle;

	friend class CoreFactory;
	template<class _FinalX,class _DomainX,class _OutputTypeX,class _Input1X,class _Input2X> friend class ObjectBase;
};

template<u32 _Index> class _VoidInput
{
public:
	typedef void	Domain;
};

template<class _Domain,u32 _Index> class _ObjectBaseInput< _Domain, _VoidInput<_Index> >
{
protected:
	//empty
	typedef void	Domain;
	typedef void	Source;
	typedef _VoidInput<_Index>	SourceClass;
	typedef void	Type;
private:

	inline void Open(MemHandle h)
	{
		assert("Should never be called");
	}

	friend class CoreFactory;
	template<class _FinalX,class _DomainX,class _OutputTypeX,class _Input1X,class _Input2X> friend class ObjectBase;
};

template<class _Domain,class _OutputType> class _ObjectBaseOutput
{
protected:
	typedef _OutputType Type;
	typedef _Domain		Domain;

	_ObjectBaseOutput(const guid& objectId) : pin(OutputPin<_Domain>::pin)
	{
		handle=pin.AllocateMemBlock(sizeof(_OutputType));
		_ObjectBaseDomain<_Domain>::PostMessage(MessageObjectOutputInit(objectId,handle));
	}
	~_ObjectBaseOutput()
	{
		if(handle.isValid())
		{
			pin.FreeMemBlock(handle);
			handle = MemHandle();
		}
	}
	inline void Open(MemHandle h)
	{
		pin.ReferenceMemBlock(h);
		handle = h;
	}
	inline bool IsReady() const
	{
		return handle.isValid();
	}
	inline const _OutputType& GetOld() const
	{
		assert(handle.isValid());
		return *(const _OutputType*)pin.GetReadMem(handle,sizeof(_OutputType));
	}
	inline void Set(const _OutputType& value)
	{
		assert(handle.isValid());
		*(_OutputType*)pin.GetWriteMem(handle,sizeof(_OutputType)) = value;
	}
	inline Type* Access()
	{
		assert(handle.isValid());
		return (_OutputType*)pin.GetWriteMem(handle,sizeof(_OutputType));
	}

private:
	PinBase& pin;
	MemHandle handle;

	template<class _DomainX,class _InputX> friend class _ObjectBaseInput;
};

template<class _Domain> class _ObjectBaseOutput<_Domain,void>
{
protected:
	typedef void		Type;
	typedef void		Domain;

	//empty
	_ObjectBaseOutput(const guid& objectId)
	{
	}

	template<class _DomainX,class _InputX> friend class _ObjectBaseInput;
};

template<class _Final,class _Domain,class _OutputType = void,class _Input1 = _VoidInput<1>,class _Input2 = _VoidInput<2>> class ObjectBase :
	public	_ObjectBaseDomain<_Domain>,
	public	_ObjectBaseOutput<_Domain,_OutputType>,
	public	_ObjectBaseInput<_Domain,_Input1>,
	public	_ObjectBaseInput<_Domain,_Input2>
{
public:
	static const std::array<guid,2>& getCapabilities(){static const std::array<guid,2> val= {guidNull,guid_of<_Final>::value}; return val;}
protected:
	typedef _ObjectBaseOutput<_Domain,_OutputType> Output;
	typedef _ObjectBaseInput<_Domain,_Input1>	Input1;
	typedef _ObjectBaseInput<_Domain,_Input2>	Input2;
	typedef _Final								ClassType;
	typedef _Domain								Domain;
	typedef ObjectBase<_Final,_Domain,_OutputType,_Input1,_Input2>		BaseType;

	ObjectBase(const guid& rguid) : _ObjectBaseDomain<_Domain>(rguid),_ObjectBaseOutput<_Domain,_OutputType>(rguid) {}
	virtual ~ObjectBase(){}

private:
	virtual bool MessageProc(const MessageObjectUnknown* pMessage)
	{
		if(pMessage->GetMessageCode() == MESSAGE_OBJECT_OUTPUT_INIT)
		{
			MessageObjectOutputInit* pDetailed = (MessageObjectOutputInit*)pMessage;
			if(pDetailed->GetSource() == guid_of<typename Input1::SourceClass::Domain>::value )
				Input1::Open(pDetailed->GetHandle());
			else if(pDetailed->GetSource() == guid_of<typename Input2::SourceClass::Domain>::value )
				Input2::Open(pDetailed->GetHandle());
			else
				assert("Recieved input from source, but no handler to recieve it!\n");
			return true;
		}
		return false;
	}

	friend class CoreFactory;
	template<class _DomainX,class _InputX> friend  class _ObjectBaseInput;
	template<class _FinalX,class _DomainX,class _OutputTypeX,class _Input1X,class _Input2X> friend class ObjectBase;
};

template<class _Final,class _Domain,class _OutputType,class _Input1,class _Input2> class guid_of<ObjectBase<_Final,_Domain,_OutputType,_Input1,_Input2>> : public guid_of<_Final>
{
};

template<u32 index> class guid_of<_VoidInput<index>> : public guid_of<void>
{
};

}

#endif
