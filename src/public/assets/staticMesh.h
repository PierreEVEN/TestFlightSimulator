#pragma once
#include <filesystem>
#include <vk_mem_alloc.h>



#include "assetBase.h"
#include "engine/jobSystem/job.h"
#include "glm/glm.hpp"

namespace job_system {
	class IJobTask;
}

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec4 col;
	glm::vec3 norm;
	glm::vec3 tang;
	glm::vec3 bitang;
};

struct VertexGroup
{
	std::vector<Vertex> vertices;
};

class StaticMesh : public AssetBase
{
	friend AssetManager;
public:
	bool try_load() override
	{
		return creation_job->is_complete;
	}
protected:

	StaticMesh(const VertexGroup& in_vertices, const std::vector<uint32_t>& in_triangles);
	~StaticMesh();

private:

	VertexGroup vertices;
	std::vector<uint32_t> indices;


	VkBuffer vertex_buffer = VK_NULL_HANDLE;
	VmaAllocation vertex_buffer_allocation = VK_NULL_HANDLE;
	VmaAllocationInfo vertex_buffer_alloc_info;

	VkBuffer index_buffer = VK_NULL_HANDLE;
	VmaAllocation index_buffer_allocation = VK_NULL_HANDLE;
	VmaAllocationInfo index_buffer_alloc_info;

	std::shared_ptr<job_system::IJobTask> creation_job;
};