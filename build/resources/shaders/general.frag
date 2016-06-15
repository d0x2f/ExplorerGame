#version 140
#define PI 3.1415926535897

in vec3 vertex;
in vec4 colour;
in vec2 tex_coords;
in vec3 light_component;

out vec4 output_colour;

uniform sampler2D tex0;

void main() {
	vec4 tex = texture(tex0, tex_coords);
	output_colour = vec4(light_component, 1.0)*colour*tex;
}