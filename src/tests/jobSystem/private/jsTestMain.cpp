#include "jobSystem/job_system.h"

#define TASK for (size_t i = 0; i < 1000; ++i) {}

void tests()
{
	for (int i = 0; i < 1; ++i) {
		auto p1 = job_system::new_job([i]
			{
				//logger_log("create parent A %d", i);

				TASK;
				job_system::new_job([i]
					{
						//logger_log("create child A A %d", i);
						TASK;
						//logger_log("end child A A %d", i);
					});
				job_system::new_job([i]
					{
						//logger_log("create child A B %d", i);
						TASK;
						//logger_log("end child A B %d", i);
					});

				TASK;

				//logger_log("end parent A %d", i);
			});

		auto p2 = job_system::new_job([i]
			{
				//logger_log("create parent B %d", i);

				TASK;
				job_system::new_job([i]
					{
						//logger_log("create child B A %d", i);
						TASK;
						//logger_log("end child B A %d", i);
					});
				job_system::new_job([i]
					{
						//logger_log("create child B B %d", i);
						TASK;
						//logger_log("end child B B %d", i);
					});

				logger_log("end parent B %d", i);
			});
	}
}


int main(int argc, char* argv[]) {
	job_system::Worker::create_workers(1);

	auto p2 = job_system::new_job([]
		{
			tests();
		});

	p2->wait();
	logger_log("complete");

	job_system::Worker::destroy_workers();
}
