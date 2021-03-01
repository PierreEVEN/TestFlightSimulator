#pragma once

#include <vector>

#include "engine/jobSystem/worker.h"
#include "engine/semaphores.h"

namespace job_system {

	class IJobTask
	{
	public:
		virtual void execute() = 0;

		void push_child_task(std::shared_ptr<IJobTask> child)
		{
			++child_count;
			child_tasks.push(child);
			Worker::wake_up_worker();
		}

		std::shared_ptr<IJobTask> pop_child_task() {
			return child_tasks.pop();
		}

		void wait()
		{
			if (is_complete) return;
			else
			{
				wait_semaphore.acquire();
			}
		}
		
		static std::shared_ptr<IJobTask> find_current_parent_task();

		std::shared_ptr<IJobTask> parent_task = nullptr;
		TObjectPool<IJobTask, 256> child_tasks;
		std::counting_semaphore<> child_wait_semaphore = std::counting_semaphore<>(0);
		int32_t child_count = 0;
		std::atomic_bool is_complete = false;
		std::binary_semaphore wait_semaphore = std::binary_semaphore(0);
	};

	template <typename Lambda>
	class TJobTask : public IJobTask {
	public:
		TJobTask(Lambda&& inFunc) : func(std::forward<Lambda>(inFunc)) {}

		virtual void execute()
		{
			MEMORY_BARRIER();
			func();
			MEMORY_BARRIER();
			while (std::shared_ptr<IJobTask> task = pop_child_task())
			{
				task->execute();
			}
			if (parent_task) parent_task->child_wait_semaphore.release();
			for (int i = 0; i < child_count; ++i) child_wait_semaphore.acquire();
			is_complete = true;
			wait_semaphore.release();
		}
	private:
		Lambda func;
	};
}
