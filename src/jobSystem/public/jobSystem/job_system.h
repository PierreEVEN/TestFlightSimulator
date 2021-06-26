#pragma once

#include "job.h"
#include "worker.h"
#include <memory>

namespace job_system
{

/** Create a new job */
template <class Lambda> std::shared_ptr<TJobTask<Lambda>> new_job(Lambda&& funcLambda, bool is_orphan = false)
{
    auto job = std::make_shared<TJobTask<Lambda>>((std::forward<Lambda>(funcLambda)));

    if (is_orphan)
        Worker::push_orphan_job(job);
    else
    {
        if (auto task = IJobTask::find_current_parent_task())
        {
            job->parent_task = task;
            task->push_child_task(job);
        }
        else
            Worker::push_orphan_job(job);
    }

    return job;
}

inline void wait_children()
{
    if (auto* worker = Worker::get())
    {
        if (auto task = worker->get_current_task())
        {
            task->wait_children();
        }
    }
}
} // namespace job_system
