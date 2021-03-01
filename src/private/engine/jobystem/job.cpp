

#include "engine/jobSystem/job.h"

namespace job_system {

	std::shared_ptr<IJobTask> IJobTask::find_current_parent_task()
	{
		if (Worker* worker = Worker::get())
		{
			return worker->get_current_task();
		}
		return nullptr;
	}
}