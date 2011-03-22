#ifndef SHARED_TASK_MANAGER_H_INCLUDED
#define SHARED_TASK_MANAGER_H_INCLUDED

#include <HAGE.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <queue>

namespace HAGE {

class InputDomain;

class SharedTaskManager
{
public:
	SharedTaskManager();
	InputDomain* StartThreads();
	void StopThreads();
	void FinalizeShutdown();
	~SharedTaskManager();

	struct workerThreadProc
	{
		SharedTaskManager* 	pTaskManager;
		i32				nCore;
		u32				nThreadId;
		boost::barrier& initbarrier;
		boost::barrier& inputbarrier;
		boost::barrier& shutdownbarrier;

		void operator () ();
	};

	struct TaskEntry
	{
		TaskEntry(u32 priority, u64 stage, u64 code);

		u32 priority;
		u64 stage;
		u64 queueCode;

		bool operator <(const TaskEntry& other) const;
	};

	TaskManager* getNextTask(TaskManager::genericTask** ppTask);
	TaskManager* getNextTaskNoBlock(TaskManager::genericTask** ppTask, u32 Priority);

	TaskManager::genericTask* GetNextTaskSpecific(u64 step,u64 queueCode,TaskManager* pManager);
	result QueueDomain(TaskManager* pTaskManager, u64 queueCode);
	void	Shutdown();
	bool IsShutdown(t64 time)
	{
		if(bShutdown && time > nShutdownTime)
			return true;
		else
			return false;
	}

	volatile i32 UserlandShutdownCounter();
	
	void InitSysUserland();
	void InitUserland();
	void EndUserland();
	
	SharedDomainBase* ConstructDomain(HAGE::u32 size);

private:
	void DestructDomains();
	void DestructDomain(SharedDomainBase* p);

	typedef std::multimap<TaskEntry,TaskManager*,std::less<TaskEntry>,global_allocator<TaskEntry> > jobqueue;

	volatile int						nSleepingThreads;
	boost::thread_group 				workerThreads;
	boost::condition_variable 			condTask;
	boost::mutex 						mutexTask;

	jobqueue							taskList;

	float								fSteptime;
	volatile bool						bShutdown;
	boost::posix_time::ptime			lastTick;

	IMain*								m_pMain;
	const int							m_nThreads;
	boost::barrier						m_initbarrier;
	boost::barrier						m_shutdownbarrier;
	volatile i32						m_userlandShutdownCounter;

	std::vector<SharedDomainBase*>		m_DomainsToDestruct;

	t64									nShutdownTime;
};

}


#endif
