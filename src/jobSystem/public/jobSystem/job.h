#pragma once

#include <vector>
#include <latch>

#include "jobSystem/worker.h"
#include "types/semaphores.h"

namespace job_system {
	
	class worker_lock
	{
	public:
		explicit worker_lock(const std::ptrdiff_t current = 0)
			: val(current) {
		}

		void count_up()
		{
			++val;
		}


		void count_down()
		{
			--val;
			try_release();
		}

		void wait()
		{
			if (val == 0) return;
			std::unique_lock<std::mutex> lock(wait_m);
			wait_cv.wait(lock);
		}
	private:

		void try_release()
		{
			if (val == 0) wait_cv.notify_all();
		}

		std::mutex wait_m;
		std::condition_variable wait_cv;
		std::atomic_ptrdiff_t val;
	};


	
	class IJobTask
	{
	public:

		virtual void execute() = 0;

		void push_child_task(const std::shared_ptr<IJobTask>& child)
		{
			child_lock.count_up();
			children_pool.push(child);
			Worker::wake_up_worker();
		}

		std::shared_ptr<IJobTask> steal_task() {
			auto task = children_pool.pop();
			return task;
		}


		void wait_children()
		{
			while (auto task = children_pool.pop()) {
				Worker::get()->current_task = task;
				task->execute();
				Worker::get()->current_task = task->parent_task;
			}
			child_lock.wait();
		}
		
		void wait()
		{
			while (auto task = children_pool.pop()) {
				Worker::get()->current_task = task;
				task->execute();
				Worker::get()->current_task = task->parent_task;
			}
			completion_lock.wait();
			child_lock.wait();
		}
		
		static std::shared_ptr<IJobTask> find_current_parent_task();		
		static int64_t get_stat_total_job_count();
		static int64_t get_stat_awaiting_job_count();
		
		[[nodiscard]] bool is_complete() const { return complete; }

		std::shared_ptr<IJobTask> parent_task = nullptr;
		TObjectPool<IJobTask, 4096> children_pool;
		worker_lock child_lock;
		std::latch completion_lock = std::latch(1);
	protected:

		void inc_job_count();
		void dec_awaiting_job_count();
		void dec_total_job_count();
		std::atomic_bool complete = false;

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
			dec_awaiting_job_count(); // stats
			func(); // execute task
			completion_lock.count_down();
			dec_total_job_count(); // stats

			wait();

			complete = true;
			if (parent_task) parent_task->child_lock.count_down();
		}
	private:
		Lambda func;
	};
}
