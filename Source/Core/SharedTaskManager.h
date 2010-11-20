#ifndef SHARED_TASK_MANAGER_H_INCLUDED
#define SHARED_TASK_MANAGER_H_INCLUDED

#include <HAGE.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <queue>

namespace HAGE {

class SharedTaskManager
{
public:
	SharedTaskManager();
	void Run();
	~SharedTaskManager();



	struct workerThreadProc
	{
		SharedTaskManager* 	pTaskManager;
		i32				nCore;
		u32				nThreadId;
		boost::barrier& initbarrier;

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
	bool IsShutdown(u64 step)
	{
		if(bShutdown && step >= nShutdownStep)
			return true;
		else
			return false;
	}


private:

	typedef std::multimap<TaskEntry,TaskManager*,std::less<TaskEntry>,global_allocator<TaskEntry> > jobqueue;

	volatile int						nSleepingThreads;
	boost::thread_group 				workerThreads;
	boost::condition_variable 			condTask;
	boost::mutex 						mutexTask;

	jobqueue							taskList;

	float								fSteptime;
	bool								bShutdown;
	boost::posix_time::ptime			lastTick;

	u64									nShutdownStep;
	u64									nHighestStep;
};

}


#endif
