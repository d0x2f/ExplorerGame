#version 140
#extension GL_ARB_explicit_attrib_location : enable
#define PI  3.1415926535897
#define PI2 6.2831853071795

layout (location = 0) in vec3 _vertex;
layout (location = 1) in vec3 _normal;
layout (location = 2) in vec4 _colour;
layout (location = 3) in vec2 _tex_coords;

out vec4 colour;
out vec3 normal;
out vec3 vertex;
out vec2 tex_coords;

uniform mat4 modelviewprojection;

//between 0 and 1, used to emulate waves
uniform float time;

void main() {
	colour = _colour;
	tex_coords = _tex_coords;
	normal = _normal;
	vertex = _vertex;
	
	float ang = vertex.x - PI2*time;
	vertex.z += sin(ang)*0.1;

	gl_Position = modelviewprojection * vec4(vertex,1.0);
}	