#version 140
#define PI 3.1415926535897
#define PI2 6.2831853071795

in vec3 vertex;
in vec3 normal;
in vec4 colour;
in vec2 tex_coords;
in vec3 light_component;

out vec4 output_colour;

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

uniform sampler2D tex0;
uniform mat4 modelview;
uniform mat3 normalmatrix;

//between 0 and 1, used to emulate waves
uniform float time;

void getEyeSpace( out vec3 norm, out vec4 position ) {
	position = modelview * vec4(vertex,1.0);
	float ang = mod(position.x - PI2*time, PI2);
	float c = cos(ang);
	float s = sin(ang);

	mat3 rotation;
	rotation[0] = vec3(c, 0, -s);
	rotation[1] = vec3(0, 1, 0);
	rotation[2] = vec3(s, 0, c);
	
	norm = normalize(normalmatrix * ((rotation * normal)*0.1 + normal*0.9));
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
	return ambient + diffuse + spec;
	//return diffuse;
}

void main() {
	vec3 eyeNorm;
	vec4 eyePosition;
	vec4 tex = texture(tex0, tex_coords);
	
	getEyeSpace(eyeNorm, eyePosition);
	vec3 _light_component = phongModel(eyePosition, eyeNorm);
	output_colour = vec4(_light_component, 1.0)*colour*tex;
}