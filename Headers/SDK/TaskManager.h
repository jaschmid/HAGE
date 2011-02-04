/********************************************************/
/* FILE: TaskManager.h                                  */
/* DESCRIPTION: Defines TaskManager class for Domains.  */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef TASK_MANAGER_H_INCLUDED
#define TASK_MANAGER_H_INCLUDED

#include "HAGE.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/random.hpp>
#include <vector>

namespace HAGE {

class SharedTaskManager;
class SharedDomainBase;

class TaskManager
{
public:
	
	class genericTask : public IRandomSource
	{
	public:
		virtual u32 GetRandInt() {return (u32)(gen()&0xffffffff);}
		virtual f32 GetRandFloat() {return (f32)(((f64)gen()-(f64)gen.min())/((f64)gen.max()-(f64)gen.min()));}
		virtual void operator() () = 0;
		virtual ~genericTask() {}
	private:
		typedef boost::rand48 generator_type;
		generator_type gen;

		void SetSeed(u32 seed){gen.seed(seed);}

		friend class TaskManager;
	};

	TaskManager();
	~TaskManager();

	template<class _T> void SetDomain(_T* p)
	{
		pGuid = &guid_of<_T>::Get();
		pDomain = (IDomain*)domain_access<_T>::Get();
	}

	void Shutdown();

	void QueueTask(genericTask* pTask);
	void Execute();

	static SharedDomainBase* ConstructDomain(HAGE::u32 size);

	enum
	{
		PRIORITY_SHUTDOWN = 0x0000,
		PRIORITY_TASK = 0x1000,
		PRIORITY_STEP = 0x2000,
		PRIORITY_INIT = 0x3000
	};

private:

	class InitTask : public genericTask
	{
	public:
		InitTask(TaskManager* pParent) : pTask(pParent)
		{
		}

		virtual void operator() ()
		{
			pTask->InitTask();
		}
		// this is an ST_TASK thus we use the domains random generator
		virtual u32 GetRandInt() {return pTask->GetRandInt();}
		virtual f32 GetRandFloat() {return pTask->GetRandFloat();}
	private:
		TaskManager* pTask;
	};

	class StepTask : public genericTask
	{
	public:
		StepTask(TaskManager* pParent) : pTask(pParent)
		{
		}

		virtual void operator() ()
		{
			pTask->StepTask();
		}
		// this is an ST_TASK thus we use the domains random generator
		virtual u32 GetRandInt() {return pTask->GetRandInt();}
		virtual f32 GetRandFloat() {return pTask->GetRandFloat();}
	private:
		TaskManager* pTask;
	};

	class ShutdownTask : public genericTask
	{
	public:
		ShutdownTask(TaskManager* pParent) : pTask(pParent)
		{
		}

		virtual void operator() ()
		{
			pTask->ShutdownTask();
			pTask->bShutdownComplete = true;
			//no notification so we don't requeue
		}
		// this is an ST_TASK thus we use the domains random generator
		virtual u32 GetRandInt() {return pTask->GetRandInt();}
		virtual f32 GetRandFloat() {return pTask->GetRandFloat();}
	private:
		TaskManager* pTask;
	};

	// internal high precision random generator funcions
	u32 GetRandInt() {return (u32)(_randomGenerator()&0xffffffff);}
	f32 GetRandFloat() {return (f32)(((f64)_randomGenerator()-(f64)_randomGenerator.min())/((f64)_randomGenerator.max()-(f64)_randomGenerator.min()));}
	void InitHighPrecisionGenerator(u64 seed){_randomGenerator.seed(seed);}

	// functions for SharedTaskMannager
	void _InternalRunTask(genericTask* pTask);

	u32 GetCurrentStep();
	u32 GetCurrentPriority();

	bool GetNextTask(genericTask** pTaskOut);

	void NotifyTaskCompleted();

	std::vector<genericTask*,global_allocator<genericTask*> > taskList;
	volatile i32 nNextTask;
	volatile i32 nTasksCompleted;
	volatile i32 nTasksToComplete;
	volatile bool bInStage;

	volatile u32 nPriority;
	volatile u32 nStep;
	volatile u32 Mode;

	boost::mt19937	_randomGenerator;

	void QueueDomainStep();
	void QueueDomainInit();

	volatile u64 nQueueCode;
	volatile bool bShutdownComplete;

	static void Initialize();

	friend void __InternalHAGEMain();

	friend class StepTask;
	friend class ShutdownTask;
	friend class InitTask;
	friend class CoreFactory;

	boost::mutex	mutex;

	InitTask	initTask;
	void InitTask();
	StepTask	stepTask;
	void StepTask();
	ShutdownTask shutdownTask;
	void ShutdownTask();

	IDomain*	pDomain;
	const guid*	pGuid;
	static SharedTaskManager* pSharedManager;
	friend class SharedTaskManager;
	friend class SharedDomainBase;
};

}

#endif
