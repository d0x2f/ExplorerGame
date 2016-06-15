#include <stdlib.h>

#include "GLShader.h"
 
GLShader::GLShader(std::string fragment_code, std::string vertex_code) : fragment_code(fragment_code), vertex_code(vertex_code) {
	initialised = false;
}

GLShader::~GLShader() {
	if(initialised) {
		glDeleteProgram(program_id);
		std::cout << "Program Deleted: " << program_id << std::endl;
	}
}
 
 void GLShader::initialise() {
	GLuint frag_id, vert_id;
	frag_id = glCreateShader(GL_FRAGMENT_SHADER);
	vert_id = glCreateShader(GL_VERTEX_SHADER);
	
	const char *f = fragment_code.c_str();
	const char *v = vertex_code.c_str();
	
	glShaderSource(frag_id, 1, &f, NULL);
	glShaderSource(vert_id, 1, &v, NULL);
	
	glCompileShader(frag_id);
	glCompileShader(vert_id);

	printShaderInfoLog(frag_id);
	printShaderInfoLog(vert_id);

	program_id = glCreateProgram();
	glAttachShader(program_id, frag_id);
	glAttachShader(program_id, vert_id);

	glLinkProgram(program_id);
	printProgramInfoLog(program_id);
	
	//Shaders can be detached and deleted after linking
	glDetachShader(program_id, frag_id);
	glDetachShader(program_id, vert_id);
	
	glDeleteShader(frag_id);
	glDeleteShader(vert_id);
	
	std::cout << "Created Shader: " << program_id << std::endl;
	checkOGLError(__FILE__, __LINE__);
	
	initialised = true;
}

GLuint GLShader::getId() {
	return program_id;
}

GLuint GLShader::getUniformLocation(std::string name) {
	std::map<std::string, GLint>::iterator it;
	
	it = uniform_location_cache.find(name);
	GLint ptr;
	if(it == uniform_location_cache.end()) {
		ptr = glGetUniformLocation(program_id, name.c_str());
		uniform_location_cache[name] = ptr;
	} else {
		ptr = it->second;
	}
	return ptr;
}

void GLShader::setUniform(std::string name, const GLfloat value) {
	std::map<std::string, GLfloat>::iterator found = uniform_float_contents_cache.find(name);
	if(found != uniform_float_contents_cache.end() && found->second == value)
		return;

	int ptr = getUniformLocation(name);
	
	if(ptr != -1) {
		glUniform1fv(ptr, 1, &value);
		uniform_float_contents_cache[name] = value;
	} else {
		#if DEBUG == 1
		std::cout << "Couldn't get uniform location of \"" << name << "\"" << std::endl;
		#endif
	}
}

void GLShader::setUniform(std::string name, const GLint value) {
	std::map<std::string, GLint>::iterator found = uniform_int_contents_cache.find(name);
	if(found != uniform_int_contents_cache.end() && found->second == value)
		return;
		
	int ptr = getUniformLocation(name);
	
	if(ptr != -1) {
		glUniform1i(ptr, value);
		uniform_int_contents_cache[name] = value;
	} else {
		#if DEBUG == 1
		std::cout << "Couldn't get uniform location of \"" << name << "\"" << std::endl;
		#endif
	}
}

void GLShader::setUniform(std::string name, const GLfloat *matrix) {
	int ptr = getUniformLocation(name);
	
	if(ptr != -1) {
		glUniformMatrix4fv(ptr, 1, GL_FALSE, matrix);
	} else {
		#if DEBUG == 1
		std::cout << "Couldn't get uniform location of \"" << name << "\"" << std::endl;
		#endif
	}
}

void GLShader::setUniformMatrix3(std::string name, const GLfloat *matrix) {
	int ptr = getUniformLocation(name);
	
	if(ptr != -1) {
		glUniformMatrix3fv(ptr, 1, GL_FALSE, matrix);
	} else {
		#if DEBUG == 1
		std::cout << "Couldn't get uniform location of \"" << name << "\"" << std::endl;
		#endif
	}
}

void GLShader::setUniform(std::string name, const GLfloat f1, const GLfloat f2, const GLfloat f3) {
	int ptr = getUniformLocation(name);
	if(ptr != -1) {
		glUniform3f(ptr, f1, f2, f3);
	} else {
		#if DEBUG == 1
		std::cout << "Couldn't get uniform location of \"" << name << "\"" << std::endl;
		#endif
	}
}

void GLShader::setUniform(std::string name, const GLfloat f1, const GLfloat f2, const GLfloat f3, const GLfloat f4) {
	int ptr = getUniformLocation(name);
	if(ptr != -1) {
		glUniform4f(ptr, f1, f2, f3, f4);
	} else {
		#if DEBUG == 1
		std::cout << "Couldn't get uniform location of \"" << name << "\"" << std::endl;
		#endif
	}
}

void GLShader::getUniform(std::string name, GLfloat *arr) {
	std::cout << "GET UNIFORM CALLED" << std::endl;
	int ptr = getUniformLocation(name);
	if(ptr != -1) {
		glGetUniformfv(program_id, ptr, arr);
	} else {
		#if DEBUG == 1
		std::cout << "Couldn't get uniform location of \"" << name << "\"" << std::endl;
		#endif
	}
}
 
void GLShader::use() {
	if(initialised)
		glUseProgram(program_id);
}
 
void GLShader::printShaderInfoLog(GLuint obj) {
#if DEBUG == 1
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		std::cout << infoLog << std::endl;
        free(infoLog);
    }
#endif
}

void GLShader::printProgramInfoLog(GLuint obj) {
#if DEBUG == 1
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		std::cout << infoLog << std::endl;
        free(infoLog);
    }
#endif
}