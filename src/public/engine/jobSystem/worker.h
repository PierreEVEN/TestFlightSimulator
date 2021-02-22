#pragma once

#include <thread>

#include "engine/containers/objectPool.h"


namespace job_system {

	class Worker {
	public:

		static void create_workers(int worker_count = -1);
		static Worker& get();

		static void push_job(class IJobTask* newTask);
	
	private:

		Worker() : worker_thread([]() { while (true) get().next_task(); } ) {}

		void next_task();

		explicit operator bool() const { return std::this_thread::get_id() == worker_thread.get_id(); }
		[[nodiscard]] std::thread::id get_thread_id() const { return worker_thread.get_id(); }

		const std::thread worker_thread;
		std::mutex ReleaseThreadsMutex;
		std::mutex WaitNewJobMutex;

		inline static std::condition_variable wake_up_worker_condition_variable;
	};
}