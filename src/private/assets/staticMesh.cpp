

#include "assets/staticMesh.h"


StaticMesh::StaticMesh(Window* context, const AssetRef& asset_reference, const std::vector<Vertex>& in_vertices,
const std::vector<uint32_t>& in_triangles)
	: GraphicResource(context, asset_reference)
{

}
