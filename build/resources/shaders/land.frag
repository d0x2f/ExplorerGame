#version 140
#define PI 3.1415926535897

in vec3 vertex;
in vec4 colour;
in vec2 tex_coords;
in vec3 light_component;

out vec4 output_colour;

uniform sampler2D tex0;
uniform sampler2D tex1;

void main() {
	vec4 tex = texture(tex0, tex_coords) * (clamp(vertex.z, -0.2, 0)*(1/0.2)+1);
	tex += texture(tex1, tex_coords) * (1-(clamp(vertex.z, -0.2, 0)*(1/0.2)+1));
	
	output_colour = vec4(light_component, 1.0)*colour*tex;
}