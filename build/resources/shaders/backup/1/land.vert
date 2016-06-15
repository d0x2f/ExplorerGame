#version 140
#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 _vertex;
layout (location = 1) in vec3 _normal;
layout (location = 2) in vec4 _colour;
layout (location = 3) in vec2 _tex_coords;

out vec3 normal;
out vec4 vertex;
out vec4 colour;
out vec2 tex_coords;
out vec3 _light_pos;

uniform mat4 projection;
uniform mat4 modelview;

void main(void) {
	vertex = modelview * vec4(_vertex, 1.0);
	normal = _normal;
	tex_coords = _tex_coords;
	colour = _colour;

	gl_Position = projection * vertex;
}