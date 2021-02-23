#pragma once

#include <thread>
#include <semaphore>

#include "engine/containers/objectPool.h"


namespace job_system {

	class Worker final {
	public:

		static void create_workers(int worker_count = -1);
		static Worker& get();

		static void push_job(class IJobTask* newTask);
		static void wait_job_completion();

	private:

		Worker();

		void next_task();

		explicit operator bool() const { return std::this_thread::get_id() == worker_thread.get_id(); }
		[[nodiscard]] std::thread::id get_thread_id() const { return worker_thread.get_id(); }

		
		const std::thread worker_thread;
		std::mutex WaitNewJobMutex;
	};
}