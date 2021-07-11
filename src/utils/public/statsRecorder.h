#pragma once

#define BEGIN_RECORD() StatRecorder __lambda_stat_recorder("", ##__FUNCTION__)
#define END_RECORD() __lambda_stat_recorder.end()

#define ADD_TIMEPOINT() { if (Profiler::get().is_profiler_recording()) StatRecorder __lambda_named_timepoint("", ##__FUNCTION__, true); }

#define ADD_NAMED_TIMEPOINT(name)  { if (Profiler::get().is_profiler_recording()) StatRecorder name(#name, ##__FUNCTION__, true); }

#define BEGIN_NAMED_RECORD(name) StatRecorder name(#name, ##__FUNCTION__)
#define END_NAMED_RECORD(name) name.end()

#include <chrono>
#include <mutex>
#include <thread>
#include <forward_list>

typedef std::chrono::steady_clock record_clock;

class StatRecorder final
{
public:

	StatRecorder(const char* name, const char* function_name, bool auto_close = false);
	~StatRecorder();
	
	void end();

private:
	record_clock::time_point start_time;
	const char* recorder_name;
	const char* recorder_function_name;
	bool has_ended;
	std::thread::id thread_id;
};

class Profiler final
{
	friend StatRecorder;
public:

	struct Stat
	{
		const char* name;
		const char* function_name;
		record_clock::time_point date;
		record_clock::duration duration;
		std::thread::id thread;
	};
	
	explicit Profiler(bool auto_record);
	~Profiler();

	static Profiler& get();
	
	void begin_record(bool silent = false);
	void end_record();

	[[nodiscard]] bool is_profiler_recording() const { return is_recording; }

	[[nodiscard]] record_clock::duration get_elapsed_time() const { return record_clock::now() - record_start; }
	[[nodiscard]] std::forward_list<Stat> get_last_result() const { return last_result; }

private:
	std::mutex access_lock;
	bool is_recording;
	void push_stat(Stat stat);
	void store_stats();
	record_clock::time_point profiler_creation_time;
	record_clock::time_point record_start;
	std::forward_list<Stat> history;
	std::forward_list<Stat> last_result;
};