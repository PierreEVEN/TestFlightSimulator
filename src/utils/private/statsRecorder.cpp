
#include "statsRecorder.h"


#include <filesystem>
#include <fstream>
#include <iostream>



#include "config.h"
#include <cpputils/logger.hpp>
#include <cpputils/stringutils.hpp>

#if _DEBUG
Profiler profiler_instance(true);
#else
Profiler profiler_instance(false);
#endif

StatRecorder::StatRecorder(const char* name, const char* function_name, bool auto_close)
	: recorder_name(name), recorder_function_name(function_name), has_ended(false), thread_id(std::this_thread::get_id())
{
	start_time = record_clock::now();
	if (auto_close) end();
}

StatRecorder::~StatRecorder()
{
	end();
}

void StatRecorder::end()
{
	if (has_ended) return;
	has_ended = true;

	Profiler::get().push_stat(Profiler::Stat {
		.name = recorder_name,
		.function_name = recorder_function_name,
		.date = start_time,
		.duration = record_clock::now() - start_time,
		.thread = thread_id,
	});
}

Profiler::Profiler(bool auto_record)
	: profiler_creation_time(record_clock::now())
{
	if (auto_record) begin_record(true);
}

Profiler::~Profiler()
{
	end_record();
}

Profiler& Profiler::get()
{
	return profiler_instance;
}

void Profiler::begin_record(bool silent)
{
	if (is_recording) return;
	std::lock_guard<std::mutex> lock(access_lock);
        if (!silent) LOG_INFO("begin profiler record");
	is_recording = true;
	record_start = record_clock::now();
}

void Profiler::end_record()
{
	if (!is_recording) return;
	std::lock_guard<std::mutex> lock(access_lock);
        LOG_INFO("storing profiler stats");
	is_recording = false;
	store_stats();
}

void Profiler::push_stat(Stat stat)
{
	if (!is_recording) return;
	std::lock_guard<std::mutex> lock(access_lock);
	history.push_front(stat);
}

void Profiler::store_stats()
{
	/**
	 * get time string
	 */
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
#if _WIN32
	localtime_s(&tstruct, &now);
#else
	localtime_r(&now, &tstruct);
#endif
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);


	std::filesystem::create_directories(config::profiler_storage_path);
	
	
	std::ofstream output(std::string(config::profiler_storage_path) + "/Profiler-" + buf + ".csv");
	if (!output)
	{ LOG_FATAL("cannot write profiler results ");
	}

	
	output << "thread, name, function, start, end" << std::endl;
	
	for (const auto& stat : history)
	{
		output << stringutils::format("%x, %s, %s, %ld, %ld",
			stat.thread,
			stat.name,
			stat.function_name,
			std::chrono::duration_cast<std::chrono::microseconds>(stat.date - profiler_creation_time).count(),
			std::chrono::duration_cast<std::chrono::microseconds>(stat.duration).count()
			) << std::endl;
	}
	
	output.close();
	last_result = history;
	history.clear();
}
