#version 140
#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 _colour;
layout (location = 3) in vec2 _tex_coords;

out vec3 LightIntensity;
out vec4 colour;
out vec2 tex_coords;

struct LightInfo {
	vec4 Position;
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

uniform mat4 modelview;
uniform mat4 projection;
uniform mat3 normalmatrix;

void getEyeSpace( out vec3 norm, out vec4 position ) {
  norm = normalize( normalmatrix * normal);
  position = modelview * vec4(vertex,1.0);
}

vec3 phongModel( vec4 position, vec3 norm ) {
	vec3 s = normalize(vec3(light.Position - position));
	vec3 v = normalize(-position.xyz);
	vec3 r = reflect( -s, norm );

	float sDotN = max( dot(s,norm), 0.0 );

	vec3 ambient = light.La * material.Ka;
	vec3 diffuse = light.Ld * material.Kd * sDotN;
	vec3 spec = vec3(0.0);

	if( sDotN > 0.0 )
		spec = light.Ls * material.Ks * pow( max( dot(r,v), 0.0 ), material.Shininess );
	  
	//return material.Kd;
	//return ambient + diffuse + spec;
	return diffuse;
}

void main() {
	colour = _colour;
	tex_coords = _tex_coords;
	vec3 eyeNorm;
	vec4 eyePosition;
	
	getEyeSpace(eyeNorm, eyePosition);
	LightIntensity = phongModel( eyePosition, eyeNorm );
	
	gl_Position = modelview * projection * vec4(vertex,1.0);
}	