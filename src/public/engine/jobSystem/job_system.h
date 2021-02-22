#pragma once

#include "job.h"
#include "worker.h"
#include "ios/logger.h"

namespace job_system {

	/** Create a new job */
	template<class Lambda>
	TJobTask<Lambda>* new_job(Lambda&& funcLambda) { return new TJobTask<Lambda>(std::forward<Lambda>(funcLambda)); }
	
}
