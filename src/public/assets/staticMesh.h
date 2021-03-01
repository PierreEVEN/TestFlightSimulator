#pragma once
#include <filesystem>

#include "GraphicResource.h"

#include "glm/glm.hpp"

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec4 col;
	glm::vec3 norm;
	glm::vec3 tang;	
};

class StaticMesh : public GraphicResource
{
public:
	StaticMesh(Window* context, const AssetRef& asset_reference, const std::vector<Vertex>& in_vertices, const std::vector<uint32_t>& in_triangles);

private:

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	
	
};