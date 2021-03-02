#pragma once
#include <filesystem>


#include "assetBase.h"
#include "glm/glm.hpp"

struct VertexGroup
{
	std::vector<glm::vec3> pos;
	std::vector<glm::vec2> uv;
	std::vector<glm::vec4> col;
	std::vector<glm::vec3> norm;
	std::vector<glm::vec3> tang;
	std::vector<glm::vec3> bitang;
};

class StaticMesh : public AssetBase
{
public:
	StaticMesh(const VertexGroup& in_vertices, const std::vector<uint32_t>& in_triangles);

private:

	std::vector<VertexGroup> vertices;
	std::vector<uint32_t> indices;
	
	
};