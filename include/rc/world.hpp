#ifndef RC_WORLD_HPP
#define RC_WORLD_HPP

#include <vector>

namespace rc
{
	/*
		Block materials
	*/
	namespace material
	{
		enum material_t
		{
			EMPTY,
			GRASS
		};
	}

	/*
		Manager of the blocks in a world
	*/
	class world
	{
	public:
		world(int sx, int sy, int sz);

		void createFlatWorld(int height, material::material_t mat = material::GRASS);

		int sizeX() const;
		int sizeY() const;
		int sizeZ() const;

		void set(int x, int y, int z, material::material_t mat);

		material::material_t get(int x, int y, int z) const;

		bool is_surrounded(int x, int y, int z) const;

		int toFlatIndex(int x, int y, int z) const;

	private:
		int sx, sy, sz;
		std::vector<material::material_t> blocks;
	};
}

#endif