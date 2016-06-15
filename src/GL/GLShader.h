#ifndef GLSHADER_H
#define GLSHADER_H

#include <map>

#include "../globals.h"

class GLShader {
public:
	GLShader(std::string fragment_code, std::string vertex_code);
	~GLShader();
	
	void initialise();
	
	void setUniform(std::string name, const GLfloat value);
	void setUniform(std::string name, const GLint value);
	void setUniform(std::string name, const GLfloat *matrix);
	void setUniformMatrix3(std::string name, const GLfloat *matrix);
	void setUniform(std::string name, const GLfloat f1, const GLfloat f2, const GLfloat f3);
	void setUniform(std::string name, const GLfloat f1, const GLfloat f2, const GLfloat f3, const GLfloat f4);
	
	void getUniform(std::string name, GLfloat *arr);
	void use();
	GLuint getId();
private:
	std::string fragment_code;
	std::string vertex_code;
	std::map<std::string, GLint> uniform_location_cache;
	std::map<std::string, GLint> uniform_int_contents_cache;
	std::map<std::string, GLfloat> uniform_float_contents_cache;
	
	GLuint program_id;
	bool initialised;
	
	GLuint getUniformLocation(std::string name);
	
	void printShaderInfoLog(GLuint obj);
	void printProgramInfoLog(GLuint obj);
};

#endif