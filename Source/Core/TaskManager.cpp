#include <HAGE.h>
#include "SharedTaskManager.h"

namespace HAGE {

#pragma warning( disable: 4355)

	TaskManager::TaskManager()
		:initTask(this),stepTask(this),shutdownTask(this),nNextTask(0),
		nTasksCompleted(0),nPriority(0),nStep(0),bShutdownComplete(false),
		nQueueCode(0),bInStage(false)
	{
	}

#pragma warning( default: 4355)

	void TaskManager::QueueDomainInit()
	{
		taskList.push_back(&initTask);
		nPriority=PRIORITY_INIT;
		Mode = THREAD_MODE_ST;
		nTasksToComplete = (i32)taskList.size();
		nNextTask = 0;
		pSharedManager->QueueDomain(this,nQueueCode);
	}

	void TaskManager::Shutdown(){pSharedManager->Shutdown();}

	void TaskManager::QueueTask(genericTask* pTask)
	{
		taskList.push_back(pTask);
	}
	void TaskManager::Execute()
	{
		const guid* pDomainGuid;
		IDomain* pDomain;

		if(taskList.size() == 0)
			return;

		// go to MT mode

		pDomainGuid = TLS::domain_guid.release();
		pDomain = TLS::domain_ptr.release();
		Mode= THREAD_MODE_MT;
		TLS::mode.reset((int*)Mode);

		nPriority = PRIORITY_TASK;
		nTasksToComplete = (i32)taskList.size();
		nNextTask = 0;

		pSharedManager->QueueDomain(this,nQueueCode);

		genericTask* pTask = nullptr;

		while(Mode == THREAD_MODE_MT)
		{
			TaskManager* pManager = pSharedManager->getNextTaskNoBlock(&pTask,PRIORITY_TASK);
			if(pManager && pTask)
			{
				pManager->TaskEnter();
				(*pTask)();
				pManager->TaskLeave();
			}
		}

		//spin until other threads are done with our last tasks

		assert(taskList.size() == 0);

		nPriority = PRIORITY_STEP;

		TLS::mode.reset((int*)Mode);
		TLS::domain_guid.reset(const_cast<guid*>(pDomainGuid));
		TLS::domain_ptr.reset(pDomain);

		nTasksToComplete = 1;
		nNextTask = 1;
	}

	TaskManager::~TaskManager()
	{
	}

	u32 TaskManager::GetCurrentStep()
	{
		return nStep;
	}
	u32 TaskManager::GetCurrentPriority()
	{
		return nPriority;
	}

	bool TaskManager::GetNextTask(genericTask** pTaskOut)
	{
		i32 task=_InterlockedIncrement(&nNextTask);

		if( task > (i32)taskList.size() )
		{
			*pTaskOut = nullptr;
			return false;
		}

		*pTaskOut =  taskList[task-1];

		if( task == taskList.size() )
		{
			return false;
		}

		return true;
	}

	void TaskManager::NotifyTaskCompleted()
	{
		if( Mode == THREAD_MODE_ST && !bShutdownComplete)
		{
			// completed step make ready for next step
			up taskListSize = taskList.size();
			if(taskListSize!=0)
			{
				int a=5;
				a=a*a;
				assert(taskListSize==0);
			}
			((SharedDomainBase*)pDomain)->StepComplete();
		}
		else if( Mode == THREAD_MODE_MT)
		{
			volatile u32 toComplete = nTasksToComplete;
			volatile u32 completed=(u32) _InterlockedIncrement(&nTasksCompleted);
			assert(completed <= toComplete );
			if( completed == (i32)toComplete )
			{
				// done with all tasks
				assert(taskList.size() == nTasksToComplete);
				taskList.clear();
				++nQueueCode;

				nTasksCompleted = 0;


				Mode = THREAD_MODE_ST;
			}
		}
	}

	void TaskManager::QueueDomainStep()
	{
		if(bInStage)
		{
			int a=5;
			a=a*a;
			assert(bInStage == false);
		}
		nStep++;
		if( !pSharedManager->IsShutdown(nStep) )
		{
			// queue a step Task
			taskList.push_back(&stepTask);
			assert(taskList.size()==1);
			nTasksToComplete = (i32)taskList.size();
			nNextTask = 0;
			nPriority=PRIORITY_STEP;
		}
		else
		{
			// queue a shutdown task
			taskList.push_back(&shutdownTask);
			assert(taskList.size()==1);
			nTasksToComplete = (i32)taskList.size();
			nNextTask = 0;
			nPriority=PRIORITY_SHUTDOWN;
		}

		pSharedManager->QueueDomain(this,nQueueCode);

	}

	void TaskManager::TaskEnter()
	{
		TLS::domain_guid.reset(const_cast<guid*>(pGuid));
		TLS::domain_ptr.reset((IDomain*)pDomain);
		TLS::mode.reset((int*)Mode);
		if(TLS::mode.get() == (int*)THREAD_MODE_ST)
		{
			assert(bInStage == false);
			bInStage=true;
		}
	}

	void TaskManager::TaskLeave()
	{

		if(TLS::mode.get() == (int*)THREAD_MODE_ST)
		{
			assert(bInStage == true);
			bInStage=false;
		}
		TLS::domain_guid.release();
		TLS::domain_ptr.release();

		NotifyTaskCompleted();
	}

	void TaskManager::InitTask()
	{
		assert(taskList.size() == 1);
		taskList.clear();
		++nQueueCode;
		pDomain->Init(nStep);
	}
	void TaskManager::StepTask()
	{
		assert(taskList.size() == 1);
		taskList.clear();
		++nQueueCode;
		pDomain->Step(nStep);
	}
	void TaskManager::ShutdownTask()
	{
		assert(taskList.size() == 1);
		taskList.clear();
		++nQueueCode;
		pDomain->Shutdown(nStep);
	}

	SharedTaskManager* TaskManager::pSharedManager = nullptr;
}
