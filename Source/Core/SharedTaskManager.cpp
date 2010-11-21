#include <HAGE.h>
#include "SharedTaskManager.h"
#include "InputDomain.h"
#include <assert.h>

extern void OSMessageQueue();
extern void OSLeaveMessageQueue();

namespace HAGE {

	SharedTaskManager::SharedTaskManager() :bShutdown(false),nSleepingThreads(0),nHighestStep(0),nShutdownStep(0),m_pInputDomain(nullptr),
		m_nThreads(boost::thread::hardware_concurrency()),
		m_initbarrier(m_nThreads + 1)
	{
		HAGE::TaskManager::pSharedManager= this;
	}

	SharedTaskManager::~SharedTaskManager()
	{
	}

	volatile i32 SharedTaskManager::UserlandShutdownCounter()
	{
		return _InterlockedDecrement(&m_userlandShutdownCounter);
	}

	
	void SharedTaskManager::InitUserland()
	{		
		m_pInputDomain = new InputDomain;
		m_pMain = HAGECreateMain();
	}

	void SharedTaskManager::EndUserland()
	{
		delete m_pMain;
		delete m_pInputDomain;
	}

	InputDomain* SharedTaskManager::StartThreads()
	{
		for(int i = 0;i < m_nThreads; i++)
		{
			workerThreadProc proc = { this , i , (u32) i, m_initbarrier };
			workerThreads.create_thread<workerThreadProc>(proc);
		}

		{
			boost::lock_guard<boost::mutex> lock(mutexTask);
			fSteptime = 0.0;
			lastTick = boost::posix_time::microsec_clock::universal_time();
		}

		m_initbarrier.wait();

		// now queue the init task

		printf("%u threads init \n",m_nThreads);
		// program is running

		assert(m_pInputDomain);

		return m_pInputDomain;
	}

	void SharedTaskManager::StopThreads()
	{
		m_userlandShutdownCounter = m_nThreads;

		{
			boost::unique_lock<boost::mutex> lock(mutexTask);
			assert(!bShutdown);
			nShutdownStep = nHighestStep +1;
			bShutdown = true;
		}

		m_initbarrier.wait();

		// program is completed

		workerThreads.join_all();

		assert(m_userlandShutdownCounter == 0);

		printf("domains destroyed\n");
	}

	void SharedTaskManager::workerThreadProc::operator () ()
	{
		//boost::this_thread::bin

		if(nThreadId == 0)
			pTaskManager->InitUserland();

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

		int count;
		if((count=pTaskManager->UserlandShutdownCounter()) == 0)
			pTaskManager->EndUserland();

		// run thread
		printf("pthread %i completed as %i\n",nThreadId,count);
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

				if(*ppTask && result)
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

		{
			boost::unique_lock<boost::mutex> lock(mutexTask);
			if(!bShutdown)
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
