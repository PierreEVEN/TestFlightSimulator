#include "engine/jobSystem/worker.h"
#include "engine/jobSystem/job.h"

#include "ios/logger.h"
#include "engine/semaphores.h"

namespace job_system {
    Worker *workers = nullptr;
    size_t worker_count = 0;

    TObjectPool<IJobTask, 2048> job_pool;
    std::counting_semaphore<> workers_release_semaphore(0);
    std::binary_semaphore workers_complete_semaphore(0);
    std::atomic_int destroy_counter = 0;

    std::condition_variable wake_up_worker_condition_variable;

    int jobs = 0;

    void Worker::create_workers(int desired_worker_count) {
        if (workers) logger_fail("cannot add more workers");

        // Create one worker per CPU thread
        if (desired_worker_count <= 0) desired_worker_count = static_cast<int>(std::thread::hardware_concurrency());

        logger_log("create %d workers over %u CPU threads from thread %x", desired_worker_count, std::thread::hardware_concurrency(), std::this_thread::get_id());
        
        // Allocate workers memory
        workers = static_cast<Worker *>(malloc(desired_worker_count * sizeof(Worker)));

        // Create and release workers
        for (size_t i = 0; i < desired_worker_count; ++i) new(workers + i) Worker();
        worker_count += desired_worker_count;
        for (size_t i = 0; i < desired_worker_count; ++i) workers_release_semaphore.release();
    }

    Worker &Worker::get() {
        for (uint32_t i = 0; i < worker_count; ++i) {
            if (workers[i]) {
                return workers[i];
            }
        }
        logger_fail("failed to find worker on current thread");
        exit(EXIT_FAILURE);
    }

    void Worker::push_job(IJobTask *newTask) {
        jobs++;
        job_pool.push(newTask);
        wake_up_worker_condition_variable.notify_one();
    }

    void Worker::wait_job_completion() {
        while (!job_pool.is_empty() || jobs > 0) {
            workers_complete_semaphore.acquire();
        }
    }

    void Worker::destroy_workers() {
        logger_log("destroy workers");
        for (int i = 0; i < worker_count; ++i) {
            workers[i].run = false;
            ++destroy_counter;
        }
        wake_up_worker_condition_variable.notify_all();
    	while (destroy_counter > 0) {}
        free(workers);
    }

    size_t Worker::get_worker_count() {
        return worker_count;
    }

    Worker::Worker()
            : worker_thread([]() {
        workers_release_semaphore.acquire();
        logger_log("create worker on thread %x", std::this_thread::get_id());
        Worker *worker;
        do {
            worker = &get();
            worker->next_task();
        } while (worker->run);
        --destroy_counter;
    }) {}

    void Worker::next_task() {
        if (IJobTask *found_job = job_pool.pop()) {
            found_job->execute();
            jobs--;
            workers_complete_semaphore.release();

        } else {
            std::unique_lock<std::mutex> WakeUpWorkerLock(WaitNewJobMutex);
            wake_up_worker_condition_variable.wait(WakeUpWorkerLock);
        }
    }
}
