#include "jobSystem/worker.h"
#include "jobSystem/job.h"

#include <cpputils/logger.hpp>
#include "types/semaphores.h"
#include "statsRecorder.h"

namespace job_system {

    uint8_t get_worker_id_internal()
    {
        if (auto* worker = Worker::get())
        {
            return worker->get_worker_id();
        }
        return 255;
    }
	
    Worker *workers = nullptr;
    size_t worker_count = 0;

    TObjectPool<IJobTask, 16384> job_pool;
    std::counting_semaphore<> workers_create_semaphore(0);
    std::counting_semaphore<> workers_release_semaphore(0);
    std::counting_semaphore<> workers_destroy_semaphore(0);
    std::binary_semaphore workers_complete_semaphore(0);

    std::condition_variable wake_up_worker_condition_variable;

    std::atomic_int jobs = 0;

    void Worker::create_workers(int desired_worker_count) {

        Logger::get().set_thread_identifier(get_worker_id_internal);
    	
        if (workers) LOG_FATAL("cannot add more workers");

        // Create one worker per CPU thread
        if (desired_worker_count <= 0) desired_worker_count = static_cast<int>(std::thread::hardware_concurrency());

        LOG_INFO("create %d workers over %u CPU threads from thread %x", desired_worker_count, std::thread::hardware_concurrency(), std::this_thread::get_id());
        
        // Allocate workers memory
        workers = static_cast<Worker *>(malloc(desired_worker_count * sizeof(Worker)));

        // Create and release workers
        for (size_t i = 0; i < desired_worker_count; ++i) new(workers + i) Worker(static_cast<uint8_t>(i));
        worker_count += desired_worker_count;
        for (size_t i = 0; i < desired_worker_count; ++i) workers_release_semaphore.release();
        for (size_t i = 0; i < desired_worker_count; ++i) workers_create_semaphore.acquire();
    }

    Worker* Worker::get() {
        for (uint32_t i = 0; i < worker_count; ++i) {
            if (workers[i]) {
                return workers + i;
            }
        }
        return nullptr;
    }

    Worker* Worker::get_worker(size_t worker_id)
    {
        return workers + worker_id;
    }

    void Worker::push_orphan_job(std::shared_ptr<IJobTask> newTask) {
        job_pool.push(newTask);
        wake_up_worker_condition_variable.notify_one();
    }

    void Worker::wait_job_completion() {
        while (!job_pool.is_empty() || jobs > 0) {
            workers_complete_semaphore.acquire();
        }
    }

    void Worker::destroy_workers() {
        for (int i = 0; i < worker_count; ++i) {
            workers[i].run = false;
        }
        wake_up_worker_condition_variable.notify_all();
    	for (int i = 0; i < worker_count; ++i)
    	{
            workers_destroy_semaphore.acquire();
    	}
        LOG_INFO("no more job - destroyed workers");
        free(workers);
    }

    size_t Worker::get_worker_count() {
        return worker_count;
    }

    void Worker::wake_up_worker()
    {
        wake_up_worker_condition_variable.notify_one();
    }

    Worker::Worker(const uint8_t worker_id)
            : id(worker_id), worker_thread([]() {
        workers_release_semaphore.acquire();
              LOG_INFO("create worker on thread %x", std::this_thread::get_id());
        workers_create_semaphore.release();
        Worker *worker;
        do {
            if (worker = get()) {
                worker->next_task();
            }
            else
                LOG_FATAL("failed to find worker on current thread");
        } while (worker->run);
        workers_destroy_semaphore.release();
    }) {}

	/**
	 * Execute next worker loop
	 */
    void Worker::next_task() {
        if (auto found_job = steal_or_get_task()) {
            current_task = found_job;
            BEGIN_NAMED_RECORD(worker_execute_job);
            ++jobs;
            ADD_NAMED_TIMEPOINT(worker_begin_job);
            found_job->execute();
            ADD_NAMED_TIMEPOINT(worker_complete_job);
            --jobs;
            workers_complete_semaphore.release();         
            current_task = found_job->parent_task;
        } else {
        	// Wait next job order
            std::unique_lock<std::mutex> WakeUpWorkerLock(WaitNewJobMutex);
            wake_up_worker_condition_variable.wait(WakeUpWorkerLock);
        }
    }

    std::shared_ptr<IJobTask> Worker::steal_or_get_task()
    {
        if (auto task = steal_task()) return task;
    	return job_pool.pop();
    }

    std::shared_ptr<IJobTask> Worker::steal_task()
    {
        for (size_t i = 0; i < worker_count; ++i)
        {
            if (!workers[i].current_task) continue;
            if (auto task = workers[i].current_task->steal_task())
            {
                return task;
            }
        }
        return nullptr;
    }
}
