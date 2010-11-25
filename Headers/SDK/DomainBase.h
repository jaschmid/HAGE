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

class LockedMessageQueue;
class PinBase;
class Message;

template<class _T> const _T* DomainCreator();

template<class _Domain> class domain_access
{
public:
	static _Domain* Get() {return p;}
private:
	static _Domain* p;
	template<class _Domain> friend const _Domain* DomainCreator();
};

#define DECLARE_DOMAIN(x,l1,s1,s2,s3,x1) class x; DECLARE_CLASS_GUID_EX(x,l1,s1,s2,s3,x1,x); x* domain_access<x>::p = nullptr

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
	template<class _T> friend class DomainBase;
};

class IFactory;


template<class _T> const _T* DomainCreator()
{
	_T* r = static_cast<_T*>(TaskManager::ConstructDomain(sizeof(_T)));
	domain_access<_T>::p = r;
	TLS::mode.reset((int*)THREAD_MODE_ST);
	TLS::domain_guid.reset(const_cast<guid*>(&guid_of<_T>::value));
	TLS::domain_ptr.reset((IDomain*)r);

	_T* result = new(r) _T;

	TLS::mode.release();
	TLS::domain_guid.release();
	TLS::domain_ptr.release();

	return result;
}

template<class _T> class DomainBase : public SharedDomainBase
{
public:
	DomainBase() : SharedDomainBase(boost::function<void()>(&StaticCallback),boost::function<void(bool)>(&StaticQueueItem),guid_of<_T>::value)

	{
		guid temp = guid_of<_T>::value;
		Tasks.SetDomain((_T*)this);
		Factory.SetDomain((_T*)this);
	}
	virtual ~DomainBase()
	{
	}
protected:

	void* operator new (std::size_t size,_T* loc)
	{
		return loc;
	}

	void operator delete(void* l, _T* loc)
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
	#endif

private:

	static void StaticCallback()
	{
		domain_access<_T>::Get()->Callback();
	}

	static void StaticQueueItem(bool bInit)
	{
		domain_access<_T>::Get()->QueueItem(bInit);
	}

	friend class TaskManager;
	friend const _T* DomainCreator<_T>();
};

}

#endif
