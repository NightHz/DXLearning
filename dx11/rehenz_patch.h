#pragma once
#include "Rehenz/mesh.h"

namespace Rehenz
{
	// create mesh from obj. file
	// pps   <- v
	// color <- vn
	// uv    <- vt
	std::shared_ptr<Mesh> CreateMeshFromObjFile(const std::string& filename);
}
