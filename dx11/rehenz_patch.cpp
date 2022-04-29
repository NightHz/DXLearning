#include "rehenz_patch.h"
#include <fstream>
#include <sstream>

namespace Rehenz
{
	std::shared_ptr<Mesh> CreateMeshFromObjFile(const std::string& filename)
	{
		std::vector<Vertex> vertices;
		std::vector<int> triangles;

		std::ifstream fs(filename.c_str());
		std::vector<Point> vs;
		std::vector<UV> vts;
		std::vector<Color> vns;

		while (fs.good())
		{
			std::string line;
			std::getline(fs, line);
			std::istringstream ss(line);

			std::string str;
			ss >> str;
			if (str.length() == 0) // empty line
				continue;
			else if (str[0] == '#') // comment line
				continue;
			else if (str == "mtlib") // material // ignore
				continue;
			else if (str == "usemtl") // material // ignore
				continue;
			else if (str == "g") // group // ignore
				continue;
			else if (str == "v") // pos
			{
				Point p;
				ss >> p.x >> p.y >> p.z;
				vs.push_back(p);
			}
			else if (str == "vt") // uv
			{
				UV uv;
				ss >> uv.x >> uv.y;
				vts.push_back(uv);
			}
			else if (str == "vn") // normal
			{
				Color c;
				ss >> c.x >> c.y >> c.z;
				vns.push_back(c);
			}
			else if (str == "f") // face
			{
				for (int i = 0; i < 3; i++)
				{
					int values[3]{ 0,0,0 };

					std::string part;
					ss >> part;
					std::istringstream part_ss(part);
					for (int j = 0; j < 3; j++)
					{
						std::string value_str;
						getline(part_ss, value_str, '/');
						if (value_str.length() != 0)
							values[j] = std::stoi(value_str);
					}
					for (int j = 0; j < 3; j++)
						values[j] -= 1;

					triangles.push_back(static_cast<int>(vertices.size()));
					if (values[0] < 0 || values[0] >= vs.size()) // no pos, pass the face
						continue;
					Vertex v(vs[values[0]]);
					if (values[2] >= 0 && values[2] < vns.size())
						v.c = vns[values[2]];
					if (values[1] >= 0 && values[1] < vts.size())
						v.uv = vts[values[1]];
					vertices.push_back(v);
				}
			}
			else // other // ignore
				continue;
		}

		return std::make_shared<Mesh>(vertices, triangles);
	}
}
