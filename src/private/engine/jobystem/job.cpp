

#include "engine/jobSystem/job.h"

namespace job_system {

	int64_t stat_awaiting_jobs = 0;
	int64_t stat_total_job = 0;
	
	std::shared_ptr<IJobTask> IJobTask::find_current_parent_task()
	{
		if (Worker* worker = Worker::get())
		{
			return worker->get_current_task();
		}
		return nullptr;
	}

	int64_t IJobTask::get_stat_total_job_count()
	{
		return stat_total_job;
	}

	int64_t IJobTask::get_stat_awaiting_job_count()
	{
		return stat_awaiting_jobs;
	}

	void IJobTask::inc_job_count()
	{
		stat_total_job++;
		stat_awaiting_jobs++;
	}

	void IJobTask::dec_awaiting_job_count()
	{
		stat_awaiting_jobs--;
	}

	void IJobTask::dec_total_job_count()
	{
		stat_total_job--;
	}
}
