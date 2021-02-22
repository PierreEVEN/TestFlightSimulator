#pragma once

#include "engine/jobSystem/worker.h"

namespace job_system {

	class IJobTask
	{
	public:
		virtual void execute() = 0;
	};

	template <typename Lambda>
	class TJobTask : public IJobTask {
	public:
		TJobTask(Lambda&& inFunc) : func(std::forward<Lambda>(inFunc)) {
			Worker::push_job(this);
		}

		virtual void execute() { func(); }
	private:
		Lambda func;
	};
}