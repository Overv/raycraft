#version 400

layout(location = 0) out vec4 outColor;
in vec2 _position;

// View info
uniform vec3 viewOrigin;
uniform mat4 invProjView;

// World info
uniform usampler3D blockData;
uniform sampler2D materials;
uniform float materialCount;
uniform uint sx, sy, sz;
uniform vec4 skyColor;

// Project screen space vector in object space
vec3 unproject(vec2 coord)
{
	vec4 v = invProjView * vec4(coord, 1.0, 1.0);
	return normalize(v.xyz);
}

// Get the material of the block at the specified position in the world
uint getBlock(ivec3 coords)
{
	return texelFetch(blockData, coords, 0).x;
}

// Convert floating point position to block coordinates
// The raytracing direction is used for correction
ivec3 toBlock(vec3 pos, vec3 dir)
{
	if ((dir.x < 0.0 && pos.x < 0.001) || (dir.y < 0.0 && pos.y < 0.001) || (dir.z < 0.0 && pos.z < 0.001))
		return ivec3(-1, -1, -1);
	else
		return ivec3(int(pos.x), int(pos.y), int(pos.z));
}

// Find the position where a line and an infinite plane intersect each other
vec3 rayPlaneIntersect(vec3 linePos, vec3 lineDir, vec3 planePos, vec3 planeNormal)
{
	float d = dot((planePos - linePos), planeNormal) / dot(lineDir, planeNormal);
	if (d > 0.0)
		return linePos + d * lineDir;
	else
		return vec3(1.0 / 0.0);
}

// Find the position where a line and a cube intersect each other
bool rayCube(vec3 origin, vec3 dir, vec3 pos, vec3 size, out vec3 hitPos, out vec3 hitNormal)
{
	// Transform world so that cube is located at (0, 0, 0) to simplify math
	origin -= pos;

	float dist = 1.0 / 0.0;
	vec3 temp, final, norm;

	// Bottom
	temp = rayPlaneIntersect(origin, dir, vec3(0, 0, 0), vec3(0, 0, 1));
	if (temp.x > 0.0 && temp.x < size.x && temp.y > 0.0 && temp.y < size.y && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
		norm = vec3(0, 0, -1);
	}

	// Top
	temp = rayPlaneIntersect(origin, dir, vec3(0, 0, size.z), vec3(0, 0, 1));
	if (temp.x > 0.0 && temp.x < size.x && temp.y > 0.0 && temp.y < size.y && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
		norm = vec3(0, 0, 1);
	}

	// Left
	temp = rayPlaneIntersect(origin, dir, vec3(0, 0, 0), vec3(-1, 0, 0));
	if (temp.y > 0.0 && temp.y < size.y && temp.z > 0.0 && temp.z < size.z && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
		norm = vec3(-1, 0, 0);
	}

	// Right
	temp = rayPlaneIntersect(origin, dir, vec3(size.x, 0, 0), vec3(1, 0, 0));
	if (temp.y > 0.0 && temp.y < size.y && temp.z > 0.0 && temp.z < size.z && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
		norm = vec3(1, 0, 0);
	}

	// Rear
	temp = rayPlaneIntersect(origin, dir, vec3(0, 0, 0), vec3(0, -1, 0));
	if (temp.x > 0.0 && temp.x < size.x && temp.z > 0.0 && temp.z < size.z && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
		norm = vec3(0, -1, 0);
	}

	// Front
	temp = rayPlaneIntersect(origin, dir, vec3(0, size.y, 0), vec3(0, 1, 0));
	if (temp.x > 0.0 && temp.x < size.x && temp.z > 0.0 && temp.z < size.z && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
		norm = vec3(0, 1, 0);
	}

	hitPos = pos + final;
	hitNormal = norm;
	
	return !isinf(dist);
}

// Get the color of a block at a certain point
vec4 blockColor(ivec3 block, vec3 pos, vec3 normal)
{
	vec3 localPos = pos - block;
	float mat = float(getBlock(block) - 1) * 1.0 / materialCount;

	if (normal.z > 0.0) {
		return texture(materials, vec2(mat + localPos.x / materialCount, localPos.y / materialCount));
	} else if (normal.z < 0.0) {
		return texture(materials, vec2(mat + localPos.x / materialCount, 0.25 + localPos.y / 4.0));
	} else if (abs(normal.x) > 0.0) {
		return texture(materials, vec2(mat + localPos.y / materialCount, 0.75 - localPos.z / 4.0));
	} else {
		return texture(materials, vec2(mat + localPos.x / materialCount, 1.0 - localPos.z / 4.0));
	}
}

// Corrects the normal using the ray direction
void correctNormal(inout vec3 normal, vec3 rayDir)
{
	if (abs(normal.x) > 0.0 && sign(normal.x) == sign(rayDir.x))
		normal.x = -normal.x;
	else if (abs(normal.y) > 0.0 && sign(normal.y) == sign(rayDir.y))
		normal.y = -normal.y;
	else if (abs(normal.z) > 0.0 && sign(normal.z) == sign(rayDir.z))
		normal.z *= -1.0;
}

void main()
{
	vec3 rayDir = unproject(_position);

	// Ray tracing state
	vec3 rayPos;
	vec3 hitP;
	vec3 hitN;

	// Assuming we're outside of the world, find the first block in the world that is hit
	if (!rayCube(viewOrigin, rayDir, vec3(0, 0, 0), vec3(sx, sy, sz), hitP, hitN)) {
		outColor = skyColor;
		return;
	}

	// Iterate until the end of the world has been reached
	bool first = true;
	ivec3 coord;
	int iterations = 0;

	while (iterations < 9999 && hitP.x > -0.001 && hitP.y > -0.001 && hitP.z > -0.001 && hitP.x < float(sx) + 0.001 && hitP.y < float(sy) + 0.001 && hitP.z < float(sz) + 0.001)
	{
		if (iterations > 0) rayCube(rayPos, rayDir, coord, vec3(1, 1, 1), hitP, hitN);

		rayPos = hitP.xyz + rayDir * 0.0001;
		coord = toBlock(rayPos, rayDir);

		if (getBlock(coord) != 0) {
			// The normal is currently the one for the previous block
			// For example, if we hit the top of a block through the bottom of an empty block, we have normal = (0, 0, -1)
			correctNormal(hitN, rayDir);

			outColor = blockColor(coord, hitP, hitN);
			return;
		}

		iterations++;
	}

	outColor = skyColor;
}