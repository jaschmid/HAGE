/********************************************************/
/* FILE: DomainBase.h                                   */
/* DESCRIPTION: Defines the DomainBase class            */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __DOMAIN_BASE_H__
#define __DOMAIN_BASE_H__

#include "HAGE.h"

#include "TaskManager.h"
#include "CoreFactory.h"
#include "DomainMemory.h"

namespace HAGE {

class VoidTraits
{
};

template<class _Final> class get_traits
{
	// abstract class
};

template<> class get_traits<void> : VoidTraits
{
	// abstract class
};

template<class _Final> class guid_of<get_traits<_Final>> : public guid_of<_Final>
{
};

template<class _Final> class domain_access<get_traits<_Final>> : public domain_access<_Final>
{
};


template<class _Domain> class DomainOutputTraits
{
	public:
		typedef _Domain		Domain;		
		typedef DomainOutputTraits<Domain> Traits;
};
template<> class DomainOutputTraits<void> : public VoidTraits
{
	public:
		typedef void		Domain;
		typedef VoidTraits	Traits;
};

template<class _Domain,bool bEnabled> class DomainOutputTraitsWrapper
{
	//abstract
};

template<class _Domain> class DomainOutputTraitsWrapper<_Domain,true>
{
	public:
		typedef typename DomainOutputTraits<_Domain>::Traits	Traits;
};
template<class _Domain> class DomainOutputTraitsWrapper<_Domain,false>
{
	public:
		typedef typename DomainOutputTraits<void>::Traits	Traits;
};

template<class _Input,i32 _Delay> class InputDelay
{
};
template<class _Domain,class _Input> class DomainInputTraits
{
	public:
		typedef _Domain				    Domain;
		typedef _Input					SourceDomain;
		typedef DomainInputTraits<Domain,SourceDomain> Traits;	
		static	const i32				InputDelay = 0;
};
template<class _Domain,class _Input,i32 _Delay> class DomainInputTraits<_Domain,InputDelay<_Input,_Delay>>
{
	public:
		typedef _Domain				    Domain;
		typedef _Input					SourceDomain;
		typedef DomainInputTraits<Domain,InputDelay<_Input,_Delay>> Traits;	
		static	const i32				InputDelay = _Delay;
};

template<u32 _Index> class _VoidInput
{
public:
	typedef void	Domain;
};

template<class _Domain> class DomainInputTraits<_Domain,void>
{
	public:
		typedef void				    Domain;
		typedef void					SourceDomain;
		typedef _VoidInput<0>			Traits;
		static	const i32				InputDelay = 0;
};

template<class _Domain,u32 i> class DomainInputTraits<_Domain,_VoidInput<i>> 
{
	public:
		typedef void				    Domain;
		typedef void					SourceDomain;
		typedef _VoidInput<i>			Traits;
		static	const i32				InputDelay = 0;
};

template<class _Domain> class DomainBase;
template<class _Domain,bool bOutput = false,class _Input1 = _VoidInput<1>,class _Input2 = _VoidInput<2>> class DomainTraits
{
	public:
		typedef typename DomainOutputTraitsWrapper<_Domain,bOutput>::Traits		OutputTraits;
		typedef typename DomainInputTraits<_Domain,_Input1>::Traits			Input1Traits;
		typedef typename DomainInputTraits<_Domain,_Input2>::Traits			Input2Traits;
		typedef _Domain												Domain;
		typedef	DomainBase<_Domain>									DomainBaseType;
};

class LockedMessageQueue;
class PinBase;
class Message;

template<class _C> class _domain_access_imp
{
public:
	inline static _C* Get(){return value();}
private:
	inline static void Set(_C* new_val){value(new_val);}
	inline static _C* value(_C* new_val = nullptr)
	{
		static _C* value=nullptr;
		if(new_val)value=new_val;
		return value;
	}
	template<class _T> friend const _T* DomainCreator();
};

#define DECLARE_DOMAIN(x,l1,s1,s2,s3,x1) class x; DECLARE_CLASS_GUID_EX(x,l1,s1,s2,s3,x1,x); \
	template<> class domain_access<x> : public _domain_access_imp<x> {}
#define DEFINE_DOMAIN(x)


class SharedDomainBase :public IDomain
{
public:

	SharedDomainBase(boost::function<void()> f,boost::function<void(bool)> f2,const guid& id);
	~SharedDomainBase();

	void AddRef(){};
	void Release(){};
	result QueryInterface(guid id,void** ppInterface){return E_FAIL;};
	void* Allocate(u64 size){return Memory->Allocate(size);}
	void Free(void* p){Memory->Free(p);}
	void PostMessage(const Message& message);

	virtual void DomainStep(u64 step){};

	void GenerateShutdownMessage(){Tasks.Shutdown();}

protected:

	virtual bool MessageProc(const Message* pMessage);

	void QueueItem(bool bInit)
	{
		if(bInit)
			Tasks.QueueDomainInit();
		else
			Tasks.QueueDomainStep();
	}

	DomainMemory*	Memory;
	TaskManager		Tasks;
	CoreFactory		Factory;

private:

	void Init(u64 step);
	void Step(u64 step);
	void Shutdown(u64 step);

	void Callback();
	void StepComplete();
	void RegisterOutput(LockedMessageQueue& out);
	void RegisterInput(LockedMessageQueue& in,i32 delay = 0);
	void ProcessMessages();

	boost::function<void()> staticCallback;
	boost::function<void(bool)> staticQueue;

	LockedMessageQueue*		outputPin;
	typedef std::pair<i32,LockedMessageQueue*> inputPin;
	typedef std::vector<inputPin,global_allocator<inputPin>> inputPinArray;
	inputPinArray inputPins;

	u32				nInputCallbacks;
	u32				nDelayedInputCallbacks;
	u32				nOutputCallbacks;

	i32				nCallbacksRecieved;
	bool			bInit;
	bool			bShutdown;

	const guid		guidDomain;

	friend class TaskManager;
	friend class SharedTaskManager;
	template<class _T,class _T2> friend class InputPin;
	template<class _T> friend class OutputPin;
	template<class _Final> friend class _DomainBaseInput;
	template<class _Final> friend class _DomainBaseOutput;
	template<class _Final> friend class DomainBase;
};

class IFactory;


template<class _T> const _T* DomainCreator()
{
	_T* r = static_cast<_T*>(TaskManager::ConstructDomain(sizeof(_T)));
	domain_access<_T>::Set(r);
	TLS::mode.reset((int*)THREAD_MODE_ST);
	TLS::domain_guid.reset(const_cast<guid*>(&guid_of<_T>::Get()));
	TLS::domain_ptr.reset((IDomain*)r);

	_T* result = new(r) _T;

	TLS::mode.release();
	TLS::domain_guid.release();
	TLS::domain_ptr.release();

	return result;
}

//input pin without delay
template<class _DomainBaseInputTraits> class _DomainBaseInput
{
protected:
	typedef typename _DomainBaseInputTraits::Domain Domain;
	typedef typename _DomainBaseInputTraits::SourceDomain SourceDomain;

	_DomainBaseInput()
	{
		((SharedDomainBase*)domain_access<Domain>::Get())->RegisterInput(GetBasePin(),GetDelay());
		//printf("Register Pin: %08x as Input to %08x with delay %i\n",&GetBasePin(),(SharedDomainBase*)domain_access<Domain>::Get(),GetDelay());
	}
	template<class _C> inline static const _C* GetMem(MemHandle p)
	{
		return (const _C*)GetBasePin().GetReadMem(p,sizeof(_C));
	}
	inline static void Free(MemHandle p)
	{
		return GetBasePin().FreeMemBlock(p);
	}
	inline static const Message* GetNextMessage(const Message* message)
	{
		return GetBasePin().GetNextMessage(message);
	}
	inline static int GetDelay()
	{
		return _DomainBaseInputTraits::InputDelay;
	}

private:

	inline static PinBase& GetBasePin()
	{
		return typename get_traits<SourceDomain>::DomainBaseType::Output::GetBasePin();
	}

	
	template<class _1> friend class InputVar;
	template<class _Domain>  friend class DomainMember;
	template<class _Domain> friend class _ObjectBaseInput;
};


//void input
template<u32 i> class _DomainBaseInput<_VoidInput<i>>
{
protected:
	typedef void	Domain;
	typedef void	Source;

	_DomainBaseInput()
	{
	}
	template<class _C> inline static const _C* GetMem(MemHandle p)
	{
		assert("Attempted to GetMem on Null Input");
		return *(_C*)nullptr;
	}
	inline static void Free(MemHandle p)
	{
		assert("Attempted to Free on Null Input");
	}
	inline static const Message* GetNextMessage(const Message* message)
	{
		assert("Attempted to GetNextMessage on Null Input");
		return (Message*)nullptr;
	}
	inline static const int& GetDelay()
	{
		assert("Attempted to GetDelay on Null Input");
		return *(int*)nullptr;
	}

private:

	inline static PinBase& GetBasePin()
	{
		assert("Attempted to GetBasePin on Null Input");
		return *(PinBase*)nullptr;
	}

	
	template<class _1> friend class InputVar;
	template<class _Domain>  friend class DomainMember;
	template<class _Domain> friend class _ObjectBaseInput;
};


template<class _DomainBaseOutputTraits> class _DomainBaseOutput
{
protected:
	typedef typename _DomainBaseOutputTraits::Domain Domain;
	
	_DomainBaseOutput()
	{
		((SharedDomainBase*)domain_access<Domain>::Get())->RegisterOutput(GetBasePin());
		//printf("Register Pin: %08x as Output to %08x\n",&GetBasePin(),(SharedDomainBase*)domain_access<Domain>::Get());
	}
	template<class _C> static inline _C* GetMem(MemHandle p)
	{
		return (_C*)GetBasePin().GetWriteMem(p,sizeof(_C));
	}
	template<class _C> static inline MemHandle Alloc()
	{
		return GetBasePin().AllocateMemBlock(sizeof(_C));
	}
	inline static void Free(MemHandle p)
	{
		return GetBasePin().FreeMemBlock(p);
	}
	template<class _TX> inline static void PostMessageOut(const _TX& message)
	{
		GetBasePin().PostMessage((const Message&)message);
	}

private:

	inline static PinBase& GetBasePin()
	{
		static PinBase pin;
		return pin;
	}

	template<class _1,class _2> friend class InputPin;
	template <class _1> friend class OutputVar;
	template <class _1> friend class InputVar;
	template<class _Domain>  friend class DomainMember;
	template<class _Final> friend class _DomainBaseInput;
	template<class _Domain> friend class _ObjectBaseOutput;
	template<class _Domain> friend class _ObjectBaseInput;
};

template<> class _DomainBaseOutput<VoidTraits>
{
protected:
	typedef void Domain;
	
	_DomainBaseOutput()
	{
	}
	template<class _C> inline static _C* GetMem(MemHandle p)
	{
		assert("Attempted to GetMem on Null Output");
		return *(_C*)nullptr;
	}
	template<class _C> inline static MemHandle Alloc()
	{
		assert("Attempted to Alloc on Null Output");
		return *(MemHandle*)nullptr;
	}
	inline static void Free(MemHandle p)
	{
		assert("Attempted to Free on Null Output");
	}
	template<class _TX> inline static void PostMessage(const _TX& message)
	{
		assert("Attempted to Post Message on Null Output");
	}
	inline static PinBase& GetBasePin()
	{
		assert("Attempted to Get Pin on Null Output");
		return *(PinBase*)nullptr;
	}

private:

	template<class _1,class _2> friend class InputPin;
	template <class _1> friend class OutputVar;
	template <class _1> friend class InputVar;
	template<class _Domain>  friend class DomainMember;
	template<class _Domain> friend class _ObjectBaseOutput;
	template<class _Domain> friend class _ObjectBaseInput;
};

template<class _Final> class _DomainBaseNewOperators
{
private:

	void* operator new (std::size_t size,_Final* loc)
	{
		return loc;
	}

	void operator delete(void* l, _Final* loc)
	{
	}

	#ifdef COMPILER_GCC //fix compiler issue

	void* operator new (std::size_t size)
	{
		return nullptr;
	}

	void operator delete(void* l)
	{
	}
	
	friend class _Final;

	#endif

	friend class TaskManager;
	friend const _Final* DomainCreator<_Final>();
};

template<class _Domain> class DomainBase : 
	public	SharedDomainBase,
		//order of construction of these is important
			_DomainBaseInput<typename get_traits<_Domain>::Input1Traits>,
			_DomainBaseInput<typename get_traits<_Domain>::Input2Traits>,
			_DomainBaseOutput<typename get_traits<_Domain>::OutputTraits>
{
protected:
	typedef typename get_traits<_Domain>				Traits;
	typedef typename Traits::Domain						Domain;
	typedef typename Traits::DomainBaseType				Base;
	typedef _DomainBaseInput<typename Traits::Input1Traits> Input1;
	typedef _DomainBaseInput<typename Traits::Input2Traits> Input2;
	typedef _DomainBaseOutput<typename Traits::OutputTraits> Output;

	DomainBase() : SharedDomainBase(boost::function<void()>(&StaticCallback),boost::function<void(bool)>(&StaticQueueItem),guid_of<Domain>::Get()),
		//enforce proper order of construction
		Input1(),Input2(),Output()

	{
		Tasks.SetDomain((Domain*)this);
		Factory.SetDomain((Domain*)this);
	}
	virtual ~DomainBase()
	{
	}

	
private:

	static void StaticCallback()
	{
		domain_access<Domain>::Get()->Callback();
	}

	static void StaticQueueItem(bool bInit)
	{
		domain_access<Domain>::Get()->QueueItem(bInit);
	}

	friend class TaskManager;
	friend const Domain* DomainCreator<Domain>();
	template<class _DomainX> friend class _ObjectBaseOutput;
	template<class _DomainX> friend class _ObjectBaseInput;
	template<class _FinalX> friend class _DomainBaseInput;
};



}

#endif
