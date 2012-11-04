#version 400

layout(location = 0) out vec4 outColor;
in vec2 _position;

// View info
uniform vec3 viewOrigin;
uniform mat4 invProjView;

// World info
uniform usampler3D blockData;
uint sx, sy, sz;

// Project screen space vector in object space
vec3 unproject(vec2 coord)
{
	vec4 v = invProjView * vec4(coord, 1.0, 1.0);
	return normalize(v.xyz);
}

// Get the material of the block at the specified position in the world
uint getBlock(uint x, uint y, uint z)
{
	return texelFetch(blockData, ivec3(x, y, z), 0).x;
}

void main()
{
	float mat = float(getBlock(0, 0, 0)) * 255.0f;
	outColor = vec4(mat, mat, mat, 1.0);

	//outColor = vec4(abs(unproject(_position)), 1.0);
}