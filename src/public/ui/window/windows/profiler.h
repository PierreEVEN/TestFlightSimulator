#pragma once
#include <unordered_map>

#include "engine/statsRecorder.h"
#include "ui/window/windowBase.h"


namespace job_system {
	class Worker;
}

class ProfilerWindow : public WindowBase
{
public:

	using WindowBase::WindowBase;

protected:

	void draw_content() override;
	
private:


	void draw_thread_stats();

	void draw_profiler_history();
	
	struct ThreadInfo
	{
		std::vector<Profiler::Stat> thread_stats;
	};

	record_clock::time_point min_time;
	record_clock::time_point max_time;

	size_t step = 250;
	float scale = 5000.0;

	std::unordered_map<job_system::Worker*, std::pair<int32_t, float[100]>> usage_history;
	std::pair<int32_t, float[100]> awaiting_job_history;
	std::pair<int32_t, float[100]> total_job_history;
	int64_t max_awaiting_job, max_total_job;

	std::chrono::steady_clock::time_point last_thread_survey;
	std::unordered_map<std::thread::id, ThreadInfo> infos;
	double time_to_local(const record_clock::time_point& time);
};
