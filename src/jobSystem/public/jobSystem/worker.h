#pragma once

#include <thread>

#include "types/objectPool.h"

#define MEMORY_BARRIER() std::atomic_thread_fence(std::memory_order_seq_cst)

namespace job_system {
	class IJobTask;

	class Worker final {
	public:

		static void create_workers(int worker_count = -1);
		static Worker* get();
		static Worker* get_worker(size_t worker_id);

		static void push_orphan_job(std::shared_ptr<IJobTask> newTask);
		static void wait_job_completion();
		static void destroy_workers();

		[[nodiscard]] static size_t get_worker_count();

		[[nodiscard]] std::thread::id get_thread() const { return worker_thread.get_id(); }
		[[nodiscard]] uint8_t get_worker_id() const { return id; }
		[[nodiscard]] std::shared_ptr<IJobTask> get_current_task() const { return current_task; }

		static void wake_up_worker();

		[[nodiscard]] bool is_busy() const { return current_task != nullptr; }

		std::shared_ptr<IJobTask> current_task = nullptr;
	private:

		Worker(const uint8_t worker_id);

		void next_task();

		explicit operator bool() const { return std::this_thread::get_id() == worker_thread.get_id(); }
		[[nodiscard]] std::thread::id get_thread_id() const { return worker_thread.get_id(); }

		[[nodiscard]] static std::shared_ptr<IJobTask> steal_or_get_task();
		[[nodiscard]] static std::shared_ptr<IJobTask> steal_task();

		const std::thread worker_thread;
		std::mutex WaitNewJobMutex;
		bool run = true;
		uint8_t id;
	};
}