#include <HAGE.h>
#include "SharedTaskManager.h"

extern void OSMessageQueue(HAGE::IMain* pMain);
extern void OSLeaveMessageQueue();

namespace HAGE {

	SharedTaskManager::SharedTaskManager() :bShutdown(false),nSleepingThreads(0),nHighestStep(0),nShutdownStep(0)
	{
	}

	void SharedTaskManager::Run()
	{
		const int nThreads = boost::thread::hardware_concurrency();

		IMain*	pMain = HAGECreateMain();

		/*
		InputDomain* pInput = new InputDomain();
		LogicDomain* pLogic = new LogicDomain();
		AIDomain* pAI = new AIDomain();
		GraphicsDomain* pGraphics = new GraphicsDomain();
		RenderingDomain* pRendering = new RenderingDomain();
		SoundDomain* pSound = new SoundDomain();
		ResourceDomain* pResource = new ResourceDomain();
		*/

		printf("domains created\n");

		boost::barrier initbarrier( nThreads + 1 );

		for(int i = 0;i < nThreads; i++)
		{
			workerThreadProc proc = { this , i , (u32) i, initbarrier };
			workerThreads.create_thread<workerThreadProc>(proc);
		}

		{
			boost::lock_guard<boost::mutex> lock(mutexTask);
			fSteptime = 0.0;
			lastTick = boost::posix_time::microsec_clock::universal_time();
		}

		initbarrier.wait();

		// now queue the init task

		printf("%u threads init \n",nThreads);
		// program is running

		OSMessageQueue(pMain);

		if( !bShutdown)
		{
			boost::unique_lock<boost::mutex> lock(mutexTask);
			if(!bShutdown)
			{
				nShutdownStep = nHighestStep +1;
				bShutdown = true;
			}
		}

		initbarrier.wait();

		// program is completed

		workerThreads.join_all();

		printf("threads terminated \n");

		delete pMain;
		/*
		delete pResource;
		delete pSound;
		delete pRendering;
		delete pGraphics;
		delete pAI;
		delete pLogic;
		delete pInput;
		*/
		printf("domains destroyed\n");
	}

	void SharedTaskManager::workerThreadProc::operator () ()
	{
		//boost::this_thread::bin

		// run thread
		printf("pthread %i started \n",nThreadId);

		initbarrier.wait();


		TaskManager::genericTask* task;
		TaskManager* pManager;
		while( pManager= pTaskManager->getNextTask(&task)  )
		{
            pManager->TaskEnter();
			(*task)();
			pManager->TaskLeave();
		}


		// run thread
		printf("pthread %i completed \n",nThreadId);
		initbarrier.wait();
	}

	TaskManager* SharedTaskManager::getNextTask(TaskManager::genericTask** ppTask)
	{
		boost::unique_lock<boost::mutex> lock(mutexTask);
		while(!( taskList.size() == 0 && bShutdown ))
		{
			while(taskList.size() > 0 )
			{
				jobqueue::iterator first = taskList.begin();
				TaskManager* result=first->second;

				if(first->first.stage > nHighestStep)
						nHighestStep = first->first.stage;

				if(!result->GetNextTask(ppTask))
				{
					taskList.erase(first);
				}

				if(*ppTask)
					return result;
			}

			if( taskList.size() == 0 && !bShutdown)
			{
				++nSleepingThreads;
				condTask.wait(lock);
				--nSleepingThreads;
			}
		}
		return nullptr;
	}

	TaskManager* SharedTaskManager::getNextTaskNoBlock(TaskManager::genericTask** ppTask, u32 Priority)
	{
		boost::unique_lock<boost::mutex> lock(mutexTask);

		for(jobqueue::iterator it = taskList.begin(); it != taskList.end(); ++it)
		{
			if(it->first.priority == Priority || Priority == 0)
			{
				TaskManager* result=it->second;

				if(it->first.stage > nHighestStep)
						nHighestStep = it->first.stage;

				if(!result->GetNextTask(ppTask))
				{
					taskList.erase(it);
				}

				if(*ppTask)
					return result;
			}
		}

		return nullptr;
	}

	result SharedTaskManager::QueueDomain(TaskManager* pTaskManager,u64 queueCode)
	{
		const guid* pDomainGuid = TLS::domain_guid.release();
		IDomain* pDomain = TLS::domain_ptr.release();

		{
			boost::unique_lock<boost::mutex> lock(mutexTask);

			if( nHighestStep < pTaskManager->GetCurrentStep() )
				nHighestStep = pTaskManager->GetCurrentStep();

			taskList.insert(
				std::pair<TaskEntry,TaskManager*>(
					TaskEntry(pTaskManager->GetCurrentPriority(),pTaskManager->GetCurrentStep(),queueCode),
					pTaskManager)
				);

			if(nSleepingThreads > 0)
            {
                condTask.notify_all();
            }

		}

		TLS::domain_guid.reset(const_cast<guid*>(pDomainGuid));
		TLS::domain_ptr.reset(pDomain);

		return S_OK;
	}

	void SharedTaskManager::Shutdown()
	{

		const guid* pDomainGuid = TLS::domain_guid.release();
		IDomain* pDomain = TLS::domain_ptr.release();

		if(!bShutdown)
		{
			boost::unique_lock<boost::mutex> lock(mutexTask);
			nShutdownStep = nHighestStep +1;
			bShutdown = true;
			OSLeaveMessageQueue();
		}

		TLS::domain_guid.reset(const_cast<guid*>(pDomainGuid));
		TLS::domain_ptr.reset(pDomain);
	}

	SharedTaskManager::TaskEntry::TaskEntry(u32 pr, u64 s, u64 qc) : priority(pr),stage(s),queueCode(qc)
	{
	}

	bool SharedTaskManager::TaskEntry::operator <(const TaskEntry& other) const
	{
		if( stage < other.stage)
			return true;
		else if( stage > other.stage )
			return false;
		else
			return priority > other.priority;
	}

}
