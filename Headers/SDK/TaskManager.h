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
#include <vector>

namespace HAGE {

class SharedTaskManager;

class TaskManager
{
public:
	
	class genericTask
	{
	protected:

		template<class _T> void SetDomain()
		{
			TLS::domain_guid.reset(&_T::id);
			TLS::domain_ptr.reset(_T::pDomain);
		}
		void ClearDomain()
		{
			TLS::domain_guid.release();
			TLS::domain_ptr.release();
		}
		void SetMode(const int* mode)
		{
			TLS::mode.reset((int*)mode);
		}
	public:
		virtual void operator() () = 0;
		virtual ~genericTask() {}
	};

	TaskManager();
	~TaskManager();

	template<class _T> void SetDomain(_T* p)
	{
		pGuid = &_T::id;
		pDomain = (IDomain*)_T::pDomain;
	}

	void Shutdown();

	void QueueTask(genericTask* pTask);
	void Execute();


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
	private:
		TaskManager* pTask;
	};


	// functions for SharedTaskMannager
	void TaskEnter();
	void TaskLeave();

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

	void QueueDomainStep();
	void QueueDomainInit();

	volatile u64 nQueueCode;
	volatile bool bShutdownComplete;

	static void Initialize();

	friend void __InternalHAGEMain();

	friend class StepTask;
	friend class ShutdownTask;
	friend class InitTask;

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
	template<class _T> friend class DomainBase;
};

}

#endif
