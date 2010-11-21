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

template<class _T> class Actor;
class LockedMessageQueue;
class PinBase;
class Message;

class SharedDomainBase :public IDomain
{
public:

	SharedDomainBase(boost::function<void()> f,boost::function<void(bool)> f2,const guid& id);
	~SharedDomainBase();

	void AddRef(){};
	void Release(){};
	result QueryInterface(guid id,void** ppInterface){return E_FAIL;};
	void* Allocate(u64 size){return Memory.Allocate(size);}
	void Free(void* p){Memory.Free(p);}
	void PostMessage(const Message& message);

	virtual void DomainInit(u64 step){};
	virtual void DomainStep(u64 step){};
	virtual void DomainShutdown(u64 step){};

	void GenerateShutdownMessage(){Tasks.Shutdown();}

protected:

	virtual bool MessageProc(const Message* pMessage);

	DomainMemory	Memory;
	TaskManager		Tasks;
	CoreFactory		Factory;

private:

	virtual void Init(u64 step);
	virtual void Step(u64 step);
	virtual void Shutdown(u64 step);

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
	template<class _T,class _T2> friend class InputPin;
	template<class _T> friend class OutputPin;
	template<class _T> friend class DomainBase;
};

class IFactory;

template<class _T> class DomainBase : public SharedDomainBase
{
public:
	DomainBase() : SharedDomainBase(boost::function<void()>(&StaticCallback),boost::function<void(bool)>(&StaticQueueItem),_T::id)

	{
		pDomain=this;
		Tasks.SetDomain((_T*)this);
		Factory.SetDomain((_T*)this);
	}
	virtual ~DomainBase()
	{
	}


	static IDomain*	pDomain;

private:

	static void StaticCallback()
	{
		((_T*)pDomain)->Callback();
	}

	static void StaticQueueItem(bool bInit)
	{
		((_T*)pDomain)->QueueItem(bInit);
	}

	friend class TaskManager;

	void QueueItem(bool bInit)
	{
		if(bInit)
			Tasks.QueueDomainInit();
		else
			Tasks.QueueDomainStep();
	}
};

template<class _T> IDomain*	DomainBase<_T>::pDomain = nullptr;

}

#endif
