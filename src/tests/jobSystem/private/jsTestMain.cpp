#include "jobSystem/job_system.h"

#include <iostream>

#define TASK for (size_t i = 0; i < 1000000000; ++i) {}

void tests()
{

}


int main(int argc, char* argv[]) {
	job_system::Worker::create_workers(1);


	auto p2 = job_system::new_job([]
		{
			job_system::new_job([]
				{
					job_system::wait_children();
				});
		});

	p2->wait();
	logger_validate("complete");

	job_system::Worker::destroy_workers();
}
