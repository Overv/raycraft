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

// Find the position where a line and an infinite plane intersect each other
vec3 rayPlaneIntersect(vec3 linePos, vec3 lineDir, vec3 planePos, vec3 planeNormal)
{
	float d = dot((planePos - linePos), planeNormal) / dot(lineDir, planeNormal);
	return linePos + d * lineDir;
}

// Find the resulting color if a line intersects a cube at pos or black if there is no intersection
vec4 rayCube(vec3 origin, vec3 dir, vec3 pos)
{
	// Transform world so that cube is located at (0, 0, 0) to simplify math
	origin -= pos;

	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	float dist = 1.0 / 0.0;
	vec3 hPos;

	// Bottom
	hPos = rayPlaneIntersect(origin, dir, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0));
	if (hPos.x > 0.0 && hPos.x < 1.0 && hPos.y > 0.0 && hPos.y < 1.0 && distance(origin, hPos) < dist) {
		dist = distance(origin, hPos);
		color = vec4(0.0, 0.0, 1.0, 1.0);
	}

	// Top
	hPos = rayPlaneIntersect(origin, dir, vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, 1.0));
	if (hPos.x > 0.0 && hPos.x < 1.0 && hPos.y > 0.0 && hPos.y < 1.0 && distance(origin, hPos) < dist) {
		dist = distance(origin, hPos);
		color = vec4(0.0, 0.0, 1.0, 1.0);
	}

	// Left
	hPos = rayPlaneIntersect(origin, dir, vec3(0.0, 0.0, 0.0), vec3(-1.0, 0.0, 0.0));
	if (hPos.y > 0.0 && hPos.y < 1.0 && hPos.z > 0.0 && hPos.z < 1.0 && distance(origin, hPos) < dist) {
		dist = distance(origin, hPos);
		color = vec4(1.0, 0.0, 0.0, 1.0);
	}

	// Right
	hPos = rayPlaneIntersect(origin, dir, vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0));
	if (hPos.y > 0.0 && hPos.y < 1.0 && hPos.z > 0.0 && hPos.z < 1.0 && distance(origin, hPos) < dist) {
		dist = distance(origin, hPos);
		color = vec4(1.0, 0.0, 0.0, 1.0);
	}

	// Rear
	hPos = rayPlaneIntersect(origin, dir, vec3(0.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0));
	if (hPos.x > 0.0 && hPos.x < 1.0 && hPos.z > 0.0 && hPos.z < 1.0 && distance(origin, hPos) < dist) {
		dist = distance(origin, hPos);
		color = vec4(0.0, 1.0, 0.0, 1.0);
	}

	// Front
	hPos = rayPlaneIntersect(origin, dir, vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 0.0));
	if (hPos.x > 0.0 && hPos.x < 1.0 && hPos.z > 0.0 && hPos.z < 1.0 && distance(origin, hPos) < dist) {
		dist = distance(origin, hPos);
		color = vec4(0.0, 1.0, 0.0, 1.0);
	}

	return color;
}

void main()
{
	outColor = rayCube(viewOrigin, unproject(_position), vec3(0.0, 0.0, 0.0));
}