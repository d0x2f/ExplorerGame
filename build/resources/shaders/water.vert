#version 140
#extension GL_ARB_explicit_attrib_location : enable
#define PI  3.1415926535897
#define PI2 6.2831853071795

layout (location = 0) in vec3 _vertex;
layout (location = 1) in vec3 _normal;
layout (location = 2) in vec4 _colour;
layout (location = 3) in vec2 _tex_coords;

struct LightInfo {
	vec3 La;
	vec3 Ld;
	vec3 Ls;
};
uniform LightInfo light;

struct MaterialInfo {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
};
uniform MaterialInfo material;

out vec4 colour;
out vec2 tex_coords;
out vec3 light_component;

layout (column_major,std140) uniform matrices {
	mat4 modelviewprojection;
	mat4 modelview;
	vec4 light_position;
};

//between 0 and 1, used to emulate waves
uniform float time;

void getEyeSpace( out vec3 norm, out vec4 position ) {
	position = modelview * vec4(_vertex,1.0);
	float ang = mod(_vertex.x - PI2*time, PI2);
	float c = cos(ang);
	float s = sin(ang);

	mat3 rotation;
	rotation[0] = vec3(c, 0, s);
	rotation[1] = vec3(0, 1, 0);
	rotation[2] = vec3(-s, 0, c);
	
	norm = normalize(mat3(modelview) * ((rotation * _normal)*0.1 + _normal*0.9));
}

vec3 phongModel( vec4 position, vec3 norm ) {
	vec3 s = normalize(-position.xyz);
	vec3 v = s;
	vec3 r = reflect( -s, norm );

	float lambert = dot(s,norm);

	vec3 ambient = light.La * material.Ka;
	vec3 diffuse = vec3(0.0);
	vec3 spec = vec3(0.0);
	
	if(lambert > 0.0) {
		diffuse = light.Ld * material.Kd * lambert;
		spec = light.Ls * material.Ks * pow( max( dot(r,v), 0.0 ), material.Shininess );
	}
	
	return ambient + diffuse + spec;
}

void main() {
	colour = _colour;
	tex_coords = _tex_coords;
	vec3 vertex = _vertex;
	
	float ang = _vertex.x - PI2*time;
	vertex.z += sin(ang)*0.1;

	vec3 eyeNorm;
	vec4 eyePosition;
	
	getEyeSpace(eyeNorm, eyePosition);
	light_component = phongModel(eyePosition, eyeNorm);
	
	gl_Position = modelviewprojection * vec4(vertex,1.0);
}	