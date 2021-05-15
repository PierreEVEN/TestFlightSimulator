
#include "ui/window/windows/profiler.h"



#include "jobSystem/job.h"
#include "jobSystem/worker.h"

#include <cpputils/logger.hpp>
#include "ui/imgui/imgui.h"


double ProfilerWindow::time_to_local(const record_clock::time_point& time) {
	return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(time - min_time).count()) / scale;
}

void ProfilerWindow::draw_content()
{
	draw_thread_stats();
	ImGui::Separator();
	draw_profiler_history();
}

void ProfilerWindow::draw_thread_stats()
{



	if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_thread_survey).count() > 1000 / 60) {
		last_thread_survey = std::chrono::steady_clock::now();
		for (int i = 0; i < job_system::Worker::get_worker_count(); ++i)
		{
			bool is_busy = job_system::Worker::get_worker(i)->is_busy();
			auto& data = usage_history[job_system::Worker::get_worker(i)];

			data.second[data.first] = is_busy ? 1.f : 0.f;
			data.first = (data.first + 1) % 100;
		}
		awaiting_job_history.second[awaiting_job_history.first] = static_cast<float>(job_system::IJobTask::get_stat_awaiting_job_count());
		total_job_history.second[total_job_history.first] = static_cast<float>(job_system::IJobTask::get_stat_total_job_count());
		awaiting_job_history.first = (awaiting_job_history.first + 1) % 100;
		total_job_history.first = (total_job_history.first + 1) % 100;
		max_awaiting_job = 0;
		max_total_job = 0;
		for (auto i = 0; i < 100; ++i)
		{
			if (total_job_history.second[i] > static_cast<float>(max_total_job)) max_total_job = static_cast<int64_t>(total_job_history.second[i]);
			if (awaiting_job_history.second[i] > static_cast<float>(max_awaiting_job)) max_awaiting_job = static_cast<int64_t>(awaiting_job_history.second[i]);
		}

	}
	ImGui::PlotHistogram("##total job count", total_job_history.second, 100, total_job_history.first, 0, 0, static_cast<float>(max_total_job), ImVec2(300, 100));
	ImGui::SameLine();
	ImGui::Text("total jobs : %ld", job_system::IJobTask::get_stat_total_job_count(), job_system::IJobTask::get_stat_awaiting_job_count());

	ImGui::PlotHistogram("##awaiting job count", awaiting_job_history.second, 100, awaiting_job_history.first, 0, 0, static_cast<float>(max_awaiting_job), ImVec2(300, 100));
	ImGui::SameLine();
	ImGui::Text("awaiting jobs : %ld", job_system::IJobTask::get_stat_total_job_count(), job_system::IJobTask::get_stat_awaiting_job_count());

	ImGui::Separator();
	for (int i = 0; i < job_system::Worker::get_worker_count(); ++i)
	{
		auto& data = usage_history[job_system::Worker::get_worker(i)];
		ImGui::PlotHistogram(stringutils::format("thread #%x", job_system::Worker::get_worker(i)->get_thread()).c_str(), data.second, 100, data.first, 0, 0, 1, ImVec2(300, 20));
	}
}

void ProfilerWindow::draw_profiler_history()
{

	if (Profiler::get().is_profiler_recording())
	{
		if (ImGui::Button("end record"))
		{
			min_time = record_clock::now();
			max_time = record_clock::time_point(std::chrono::seconds(0));
			Profiler::get().end_record();
			infos.clear();
			for (auto& item : Profiler::get().get_last_result())
			{
				if (item.date < min_time) min_time = item.date;
				if (item.date > max_time) max_time = item.date;
				infos[item.thread].thread_stats.push_back(item);
			}
		}
		ImGui::SameLine();
		ImGui::Text("%fs", static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(Profiler::get().get_elapsed_time()).count()) / 1000.f);
	}
	else
	{
		if (ImGui::Button("start record"))
		{
			Profiler::get().begin_record();
		}
	}
	
	if (infos.empty()) return;

	const double length = time_to_local(record_clock::time_point(max_time));
	const size_t max_grad = std::chrono::duration_cast<std::chrono::milliseconds>(max_time - min_time).count() / step;
	ImGui::DragFloat("scale", &scale, 2, 0, 10000);

	if (ImGui::BeginChild("content_container", ImVec2(ImGui::GetContentRegionAvailWidth(), static_cast<float>(infos.size()) * (20 + 23) + 50), true, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (ImGui::BeginChild("content", ImVec2(static_cast<float>(length) + 10, static_cast<float>(infos.size()) * (20 + 23) + 20)))
		{
			for (size_t i = 0; i < max_grad; ++i)
			{
				ImGui::GetWindowDrawList()->AddText(
					ImVec2(static_cast<float>(time_to_local(record_clock::time_point(min_time + std::chrono::milliseconds(i * step)))) + ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + 10),
					IM_COL32(255, 255, 255, 255), (std::to_string(i * step) + "ms").c_str());
			}

			ImGui::Dummy(ImVec2(0, 20));

			for (const auto& thread : infos)
			{
				ImGui::Text("thread #%x", thread.first);
				if (ImGui::BeginChild(stringutils::format("child_%x", thread.first).c_str(), ImVec2(static_cast<float>(length), 20), true))
				{
					for (auto& elem : thread.second.thread_stats) {

						ImVec2 min(static_cast<float>(time_to_local(elem.date)) + ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
						ImVec2 max(static_cast<float>(time_to_local(elem.date + elem.duration)) + ImGui::GetWindowPos().x, 20 + ImGui::GetWindowPos().y);
						if (max.x < min.x + 2) max.x = min.x + 2;

						if (ImGui::IsMouseHoveringRect(min, max))
						{
							ImGui::BeginTooltip();
							ImGui::Text("%s => %s\nduration : %fms\n", elem.name, elem.function_name, std::chrono::duration_cast<std::chrono::microseconds>(elem.duration).count() / 1000.f);
							ImGui::EndTooltip();
							ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(255, 255, 0, 100));
						}
						else
						{
							ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(255, 0, 0, 100));
						}

					}
				}
				ImGui::EndChild();
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();


}
