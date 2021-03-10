#pragma once

#include <vector>

#include "jobSystem/worker.h"
#include "types/semaphores.h"

namespace job_system {

	class IJobTask
	{
	public:
		virtual void execute() = 0;

		void push_child_task(const std::shared_ptr<IJobTask>& child)
		{
			++child_count;
			children_pool.push(child);
			Worker::wake_up_worker();
		}

		std::shared_ptr<IJobTask> steal_task() {
			auto task = children_pool.pop();
			return task;
		}

		void wait()
		{
			if (is_complete) return;
			
			while (auto task = children_pool.pop()) {
				task->execute();
			}
			task_complete_semaphore.acquire();
			
		}
		
		static std::shared_ptr<IJobTask> find_current_parent_task();

		std::shared_ptr<IJobTask> parent_task = nullptr;
		TObjectPool<IJobTask, 4096> children_pool;
		std::counting_semaphore<> child_wait_semaphore = std::counting_semaphore<>(0);
		int32_t child_count = 0;
		std::atomic_bool is_complete = false;
		std::binary_semaphore task_complete_semaphore = std::binary_semaphore(0);

		static int64_t get_stat_total_job_count();
		static int64_t get_stat_awaiting_job_count();
	
	protected:

		void inc_job_count();
		void dec_awaiting_job_count();
		void dec_total_job_count();
	};

	template <typename Lambda>
	class TJobTask : public IJobTask {
	public:
		TJobTask(Lambda&& inFunc) : func(std::forward<Lambda>(inFunc))
		{
			inc_job_count();
		}

		virtual void execute()
		{
			MEMORY_BARRIER();
			dec_awaiting_job_count(); // stats
			MEMORY_BARRIER();
			func(); // execute task
			MEMORY_BARRIER();
			dec_total_job_count(); // stats
			MEMORY_BARRIER();

			// execute remaining child tasks
			while (std::shared_ptr<IJobTask> task = steal_task()) task->execute();
			if (parent_task) parent_task->child_wait_semaphore.release();
			for (int i = 0; i < child_count; ++i) child_wait_semaphore.acquire();
			is_complete = true;
			task_complete_semaphore.release();
		}
	private:
		Lambda func;
	};
}
