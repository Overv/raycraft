#version 400

layout(location = 0) out vec4 outColor;
in vec2 _position;

// View info
uniform vec3 viewOrigin;
uniform mat4 invProjView;

// World info
uniform usampler3D blockData;
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
vec4 rayCube(vec3 origin, vec3 dir, vec3 pos, vec3 size)
{
	// Transform world so that cube is located at (0, 0, 0) to simplify math
	origin -= pos;

	float dist = 1.0 / 0.0;
	vec3 temp, final;

	// Bottom
	temp = rayPlaneIntersect(origin, dir, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0));
	if (temp.x > 0.0 && temp.x < size.x && temp.y > 0.0 && temp.y < size.y && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
	}

	// Top
	temp = rayPlaneIntersect(origin, dir, vec3(0.0, 0.0, size.z), vec3(0.0, 0.0, 1.0));
	if (temp.x > 0.0 && temp.x < size.x && temp.y > 0.0 && temp.y < size.y && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
	}

	// Left
	temp = rayPlaneIntersect(origin, dir, vec3(0.0, 0.0, 0.0), vec3(-1.0, 0.0, 0.0));
	if (temp.y > 0.0 && temp.y < size.y && temp.z > 0.0 && temp.z < size.z && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
	}

	// Right
	temp = rayPlaneIntersect(origin, dir, vec3(size.x, 0.0, 0.0), vec3(1.0, 0.0, 0.0));
	if (temp.y > 0.0 && temp.y < size.y && temp.z > 0.0 && temp.z < size.z && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
	}

	// Rear
	temp = rayPlaneIntersect(origin, dir, vec3(0.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0));
	if (temp.x > 0.0 && temp.x < size.x && temp.z > 0.0 && temp.z < size.z && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
	}

	// Front
	temp = rayPlaneIntersect(origin, dir, vec3(0.0, size.y, 0.0), vec3(0.0, 1.0, 0.0));
	if (temp.x > 0.0 && temp.x < size.x && temp.z > 0.0 && temp.z < size.z && distance(origin, temp) < dist) {
		dist = distance(origin, temp);
		final = temp;
	}

	return vec4(pos + final, dist);
}

void main()
{
	vec3 rayDir = unproject(_position);

	// Assuming we're outside of the world, find the first block in the world that is hit
	vec4 hit = rayCube(viewOrigin, rayDir, vec3(0, 0, 0), vec3(sx, sy, sz));
	if (isinf(hit.w)) {
		outColor = skyColor;
		return;
	}

	// Initial iteration
	vec3 pos = hit.xyz + rayDir * 0.001;
	ivec3 coord = toBlock(pos, rayDir);

	if (getBlock(coord) != 0) {
		outColor = vec4(1.0, 1.0, 1.0, 1.0);
		return;
	}

	// Iterate until the end of the world has been reached
	while (hit.x > -0.001 && hit.y > -0.001 && hit.z > -0.001 && hit.x < float(sx) + 0.001 && hit.y < float(sy) + 0.001 && hit.z < float(sz) + 0.001)
	{
		hit = rayCube(pos, rayDir, coord, vec3(1, 1, 1));
		pos = hit.xyz + rayDir * 0.001;
		coord = toBlock(pos, rayDir);

		if (getBlock(coord) != 0) {
			outColor = vec4(1.0, 1.0, 1.0, 1.0);
			return;
		}
	}

	outColor = skyColor;
}