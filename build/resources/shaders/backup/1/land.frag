#version 140


in vec2 tex_coords;
in vec3 normal;
in vec4 vertex;
in vec4 colour;

out vec4 out_colour;

uniform sampler2D tex0;
uniform sampler2D tex1;

uniform vec3 light_pos;

void main() {
	out_colour = vec4(0.0, 0.0, 0.0, 0.0);
	
	vec3 L = normalize(light_pos - vertex.xyz); 
	vec3 E = normalize(vertex.xyz);
	vec3 R = normalize(-reflect(L,normal)); 

	vec4 ambient = vec4(0.5, 0.5, 0.5, 1.0);
	vec4 diffuse = vec4(0.5, 0.5, 0.5, 1.0) * max(dot(normal,L), 0.0);
	vec4 specular = vec4(0.1, 0.1, 0.1, 1.0) * pow(max(dot(R,E),0.0), 100);
	
	vec4 tex = texture(tex0, tex_coords);
	out_colour += tex*clamp(vertex.z+1, 0.5, 1.0);
	
	tex = texture(tex1, tex_coords);
	out_colour += tex*(1-clamp(vertex.z+1, 0.5, 1.0));
	
	out_colour *= colour;
	out_colour *= ambient + diffuse + specular;
	out_colour.w = 1;
}
