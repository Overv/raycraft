#include <rc/world.hpp>

namespace rc
{
	world::world(int sx, int sy, int sz)
	{
		this->sx = sx;
		this->sy = sy;
		this->sz = sz;
		this->blocks = new material::material_t[sx * sy * sz];
	}

	void world::createFlatWorld(int height)
	{
		for (int x = 0; x < sx; x++)
			for (int y = 0; y < sy; y++)
				for (int z = 0; z < sz; z++)
					blocks[posToIndex(x, y, z)] = z < height ? material::GRASS : material::EMPTY;
	}

	void world::set(int x, int y, int z, material::material_t mat)
	{
		if (x >= 0 && y >= 0 && z >= 0 && x < sx && y < sy && z < sz)
			blocks[posToIndex(x, y, z)] = mat;
	}

	material::material_t world::get(int x, int y, int z)
	{
		if (x >= 0 && y >= 0 && z >= 0 && x < sx && y < sy && z < sz)
			return blocks[posToIndex(x, y, z)];
		else
			return material::EMPTY;
	}

	int world::posToIndex(int x, int y, int z)
	{
		return x * sy * sz + y * sz + z;
	}
}