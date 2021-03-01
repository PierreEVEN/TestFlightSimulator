#pragma once

#include "job.h"
#include "worker.h"
#include "ios/logger.h"

namespace job_system {

	/** Create a new job */
	template<class Lambda>
	TJobTask<Lambda>* new_job(Lambda&& funcLambda, bool is_orphan = false) { return new TJobTask<Lambda>(std::forward<Lambda>(funcLambda), is_orphan); }
	
}
