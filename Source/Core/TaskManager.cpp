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
		pTask->SetSeed(pDomain->GetRandInt());
		taskList.push_back(pTask);
	}
	void TaskManager::Execute()
	{
		const guid* pDomainGuid;
		IDomain* pDomain;

		if(taskList.size() == 0)
			return;

		// go to MT mode

		TLS_data* p = TLS::getData();
		TLS_data backup = *p;

		p->domain_guid = guidNull;
		p->domain_ptr = nullptr;
		p->random_generator = nullptr;
		Mode = p->mode = THREAD_MODE_MT;

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
				pManager->_InternalRunTask(pTask);
			}
		}

		//spin until other threads are done with our last tasks

		assert(taskList.size() == 0);

		nPriority = PRIORITY_STEP;

		*p = backup;

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
		assert(bInStage == false);
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

	void TaskManager::_InternalRunTask(genericTask* pTask)
	{
		TLS_data* p = TLS::getData();
		p->domain_guid = *pGuid;
		p->domain_ptr = pDomain;
		p->mode = Mode;
		p->random_generator = pTask;
		if(p->mode == THREAD_MODE_ST)
		{
			assert(bInStage == false);
			bInStage=true;
		}
		(*pTask)();
		if(p->mode == THREAD_MODE_ST)
		{
			assert(bInStage == true);
			bInStage=false;
		}
		p->domain_guid = guidNull;
		p->domain_ptr = nullptr;
		p->random_generator = nullptr;

		NotifyTaskCompleted();
	}

	void TaskManager::InitTask()
	{
		assert(taskList.size() == 1);
		taskList.clear();
		++nQueueCode;
		static_cast<SharedDomainBase*>(pDomain)->Init();
	}
	void TaskManager::StepTask()
	{
		assert(taskList.size() == 1);
		taskList.clear();
		++nQueueCode;
		static_cast<SharedDomainBase*>(pDomain)->Step();
	}
	void TaskManager::ShutdownTask()
	{
		assert(taskList.size() == 1);
		taskList.clear();
		++nQueueCode;
		static_cast<SharedDomainBase*>(pDomain)->Shutdown();
	}

	SharedDomainBase* TaskManager::ConstructDomain(HAGE::u32 size)
	{
		return pSharedManager->ConstructDomain(size);
	}

	SharedTaskManager* TaskManager::pSharedManager = nullptr;
}
