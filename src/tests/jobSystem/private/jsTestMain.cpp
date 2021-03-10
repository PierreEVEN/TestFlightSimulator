#include "jobSystem/job_system.h"

#include <barrier>
#include <latch>

#define TASK for (size_t i = 0; i < 1000000000; ++i) {}

void tests()
{

	job_system::new_job([]
		{
			logger_log("create parent A");

			TASK

				job_system::new_job([]
					{
						logger_log("create child A A");
						TASK
							logger_log("end child A A");
					});
			job_system::new_job([]
				{
					logger_log("create child A B");
					TASK
						logger_log("end child A B");
				});

			TASK

				logger_log("end parent A");
		});

	job_system::new_job([]
		{
			logger_log("create parent B");

			TASK

				job_system::new_job([]
					{
						logger_log("create child B A");
						TASK
							logger_log("end child B A");
					});
			job_system::new_job([]
				{
					logger_log("create child B B");
					TASK
						logger_log("end child B B");
				});

			TASK

				logger_log("end parent B");
		});
}


class worker_lock
{
public:
	explicit worker_lock(const std::ptrdiff_t current)
		: val(current) {}
	
	void count_up()
	{
		++val;
	}


	void count_down()
	{
		--val;
		try_release();
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(wait_m);
		wait_cv.wait(lock);
	}
private:

	void try_release()
	{
		if (val == 0) wait_cv.notify_one();
	}
	
	std::mutex wait_m;
	std::condition_variable wait_cv;
	std::atomic_ptrdiff_t val;
};



int main(int argc, char* argv[]) {
	job_system::Worker::create_workers();
	
	worker_lock lock(0);
	
	for (int i = 0; i < 200; ++i) {
		lock.count_up();
		job_system::new_job([&]
			{
				logger_warning("arrive and wait");
				TASK
					logger_warning("complete");
				lock.count_down();
			});
		lock.count_up();
		job_system::new_job([&]
			{
				logger_warning("arrive and wait");
				TASK
					logger_warning("complete");
				lock.count_down();
			});
		lock.count_up();
		job_system::new_job([&]
			{
				logger_warning("arrive and wait");
				TASK
					logger_warning("complete");
				lock.count_down();
			});
	}
	lock.wait();
	logger_warning("drop");

	
	job_system::wait_children();	
	job_system::Worker::destroy_workers();
}
