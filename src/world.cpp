#include <rc/world.hpp>

namespace rc
{
	world::world(int sx, int sy, int sz)
	{
		this->sx = sx;
		this->sy = sy;
		this->sz = sz;
		this->blocks = std::vector<material::material_t>(sx * sy * sz);
	}

	void world::createFlatWorld(int height, material::material_t mat)
	{
		for (int x = 0; x < sx; x++)
			for (int y = 0; y < sy; y++)
				for (int z = 0; z < sz; z++)
					blocks[toFlatIndex(x, y, z)] = z < height ? mat : material::EMPTY;
	}

	int world::sizeX() const { return sx; }
	int world::sizeY() const { return sy; }
	int world::sizeZ() const { return sz; }

	void world::set(int x, int y, int z, material::material_t mat)
	{
		if (x >= 0 && y >= 0 && z >= 0 && x < sx && y < sy && z < sz)
			blocks[toFlatIndex(x, y, z)] = mat;
	}

	material::material_t world::get(int x, int y, int z) const
	{
		if (x >= 0 && y >= 0 && z >= 0 && x < sx && y < sy && z < sz)
			return blocks[toFlatIndex(x, y, z)];
		else
			return material::EMPTY;
	}

	material::material_t world::get(int i) const
	{
		if (i < sx * sy * sz)
			return blocks[i];
		else
			return material::EMPTY;
	}

	int world::toFlatIndex(int x, int y, int z) const
	{
		return z * sy * sx + y * sx + x;
	}
}