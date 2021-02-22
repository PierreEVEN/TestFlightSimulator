#include "engine/jobSystem/worker.h"
#include "engine/jobSystem/job.h"

#include "ios/logger.h"

#define MAX_WORKER_COUNT 128

namespace job_system
{
	Worker* workers = nullptr;
	size_t current_worker_count = 0;
	TObjectPool<IJobTask, 2048> job_pool;

	
	void Worker::create_workers(int worker_count) {
		if (worker_count < 0) worker_count = std::thread::hardware_concurrency();

		logger::log("create %d workers", worker_count);

		if (!workers) workers = static_cast<Worker*>(malloc(MAX_WORKER_COUNT * sizeof(Worker)));
		if (worker_count + current_worker_count > MAX_WORKER_COUNT) {
			logger::error("cannot create more than %d workers", MAX_WORKER_COUNT);
			return;
		}
		
		for (size_t i = current_worker_count; i < current_worker_count + worker_count; ++i) {
			new (workers + i) Worker();
		}

		current_worker_count += worker_count;
	}

	Worker& Worker::get()
	{
		for (uint32_t i = 0; i < current_worker_count; ++i) 
		{
			if (workers[i]) {
				return workers[i];
			}
		}
		logger::fail("failed to find worker on current thread");
		exit(EXIT_FAILURE);
	}

	void Worker::push_job(IJobTask* newTask)
	{
		job_pool.push(newTask);
	}

	void Worker::next_task()
	{
		if (IJobTask* found_job = job_pool.pop())
		{
			found_job->execute();
		}
		else
		{
			std::unique_lock<std::mutex> WakeUpWorkerLock(WaitNewJobMutex);
			wake_up_worker_condition_variable.wait(WakeUpWorkerLock);
		}
	}
}
