#version 400

layout(location = 0) out vec4 outColor;
in vec2 _position;

// View information
uniform vec3 viewOrigin;
uniform mat4 invProjView;

// Project screen space vector in object space
vec3 unproject(vec2 coord)
{
	vec4 v = invProjView * vec4(coord, 1.0, 1.0);
	return normalize(v.xyz);
}

void main()
{
	outColor = vec4(abs(unproject(_position)), 1.0);
}