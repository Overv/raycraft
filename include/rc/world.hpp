#ifndef RC_WORLD_HPP
#define RC_WORLD_HPP

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

		void createFlatWorld(int height);

		void set(int x, int y, int z, material::material_t mat);
		material::material_t get(int x, int y, int z);

	private:
		int sx, sy, sz;
		material::material_t* blocks;

		int posToIndex(int x, int y, int z);
	};
}

#endif