#include <HAGE.h>
#include "SharedTaskManager.h"
#include "InputDomain.h"
#include "ResourceDomain.h"
#include <assert.h>
#ifdef TARGET_WINDOWS
#include <Windows.h>
#undef PostMessage
#endif

extern void OSMessageQueue();
extern void OSLeaveMessageQueue();
extern void OSNotifyMessageQueueThreadsShutdown();

namespace HAGE {

	SharedTaskManager::SharedTaskManager() :bShutdown(false),nSleepingThreads(0),nHighestStep(0),nShutdownStep(0),
		m_nThreads(boost::thread::hardware_concurrency()),
		m_initbarrier(m_nThreads),
		m_shutdownbarrier(m_nThreads+1)
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

	void SharedTaskManager::InitSysUserland()
	{
		DomainCreator<ResourceDomain>();
		DomainCreator<InputDomain>();
	}

	void SharedTaskManager::InitUserland()
	{
		m_pMain = HAGECreateMain();
	}

	void SharedTaskManager::EndUserland()
	{
		delete m_pMain;
		DestructDomains();
	}

	InputDomain* SharedTaskManager::StartThreads()
	{
		boost::barrier						m_inputbarrier(2);
		for(int i = 0;i < m_nThreads; i++)
		{
			workerThreadProc proc = { this , i , (u32) i, m_initbarrier, m_inputbarrier, m_shutdownbarrier};
			workerThreads.create_thread<workerThreadProc>(proc);
		}

		{
			boost::lock_guard<boost::mutex> lock(mutexTask);
			fSteptime = 0.0;
			lastTick = boost::posix_time::microsec_clock::universal_time();
		}
		
		// now queue the init task
		m_inputbarrier.wait();

		printf("%u threads init \n",m_nThreads);
		// program is running

		return domain_access<InputDomain>::Get();
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
	}

	void SharedTaskManager::FinalizeShutdown()
	{
		m_shutdownbarrier.wait();

		// program is completed

		workerThreads.join_all();

		assert(m_userlandShutdownCounter == 0);

		printf("domains destroyed\n");
	}
	

	void SharedTaskManager::workerThreadProc::operator () ()
	{
		//boost::this_thread::bin
#ifdef TARGET_WINDOWS
		SetThreadIdealProcessor(GetCurrentThread(),nThreadId);
#endif
		TLS::Init(nThreadId);

		if(nThreadId == 0)
		{
			pTaskManager->InitSysUserland();
			inputbarrier.wait();
			pTaskManager->InitUserland();
		}

		// run thread
		printf("pthread %i started \n",nThreadId);

		initbarrier.wait();


		TaskManager::genericTask* task;
		TaskManager* pManager;
		while( pManager= pTaskManager->getNextTask(&task)  )
		{
            pManager->_InternalRunTask(task);
		}

		int count;
		if((count=pTaskManager->UserlandShutdownCounter()) == 0)
		{
			pTaskManager->EndUserland();
			OSNotifyMessageQueueThreadsShutdown();
		}

		// run thread
		printf("pthread %i completed as %i\n",nThreadId,count);
		shutdownbarrier.wait();

		TLS::Free();
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
		TLS_data* p = TLS::getData();
		TLS_data backup = *p;
		p->domain_guid = guidNull;
		p->domain_ptr = nullptr;
		p->random_generator = nullptr;

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

		*p=backup;

		return S_OK;
	}

	void SharedTaskManager::Shutdown()
	{
		
		TLS_data* p = TLS::getData();
		TLS_data backup = *p;
		p->domain_guid = guidNull;
		p->domain_ptr = nullptr;
		p->random_generator = nullptr;

		{
			boost::unique_lock<boost::mutex> lock(mutexTask);
			if(!bShutdown)
				OSLeaveMessageQueue();
		}
		
		*p=backup;
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

	SharedDomainBase* SharedTaskManager::ConstructDomain(HAGE::u32 size)
	{
		DomainMemory* pMem = new (DomainMemory::GlobalAllocate(sizeof(DomainMemory))) DomainMemory();
		SharedDomainBase* r = (SharedDomainBase*)pMem->Allocate(size);
		r->Memory = pMem;
		m_DomainsToDestruct.push_back(r);
		return r;
	}

	void SharedTaskManager::DestructDomain(SharedDomainBase* p)
	{
		DomainMemory* pMem = p->Memory;
		
		TLS_data* data = TLS::getData();
		TLS_data backup = *data;
		data->domain_guid = p->guidDomain;
		data->domain_ptr = (IDomain*)p;
		data->random_generator = (IDomain*)p;
		data->mode = THREAD_MODE_ST;

		delete(p,p);

		*data=backup;

		DomainMemory::GlobalFree(pMem);
	}

	void SharedTaskManager::DestructDomains()
	{
		for(auto i = m_DomainsToDestruct.begin();i!=m_DomainsToDestruct.end();++i)
			DestructDomain(*i);
	}

}
