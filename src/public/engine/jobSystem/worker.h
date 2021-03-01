#pragma once

#include <thread>

#include "engine/containers/objectPool.h"

#define MEMORY_BARRIER() std::atomic_thread_fence(std::memory_order_seq_cst)

namespace job_system {

	class Worker final {
	public:

		static void create_workers(int worker_count = -1);
		static Worker* get();

		static void push_orphan_job(class IJobTask* newTask);
		static void wait_job_completion();
		static void destroy_workers();

		[[nodiscard]] static size_t get_worker_count();

		[[nodiscard]] IJobTask* get_current_task() const { return current_task; }

		static void wake_up_worker();
	
	private:

		Worker();

		void next_task();

		explicit operator bool() const { return std::this_thread::get_id() == worker_thread.get_id(); }
		[[nodiscard]] std::thread::id get_thread_id() const { return worker_thread.get_id(); }

		IJobTask* current_task = nullptr;
		static IJobTask* steal_or_get_task();

		const std::thread worker_thread;
		std::mutex WaitNewJobMutex;
		bool run = true;
	};
}