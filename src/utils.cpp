#include <cstdarg>
#include <fstream>

#include "utils.h"

GLint averagei(int n, ...) {
	va_list arguments;
	GLint sum = 0;
	GLint val;
	
	va_start(arguments, n);
	
	for(int i=0; i<n; i++) {
		val = va_arg(arguments, GLint);
		
		sum += val;
	}
	
	va_end(arguments);
	
	return sum/n;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	
	return elems;
}

int ids = 100;
int getNewId() {
	return ++ids;
}

std::string readFile(const char *filename) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}

GLfloat randomFloat() {
	return (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));	
}

void matrixMultiply44(GLfloat a[16], GLfloat b[16], GLfloat ret[16]) {
	for(int i=0; i<4; i++) {
		for(int j=0; j<4; j++) {
			ret[i*4+j] = 0;
			for(int x=0; x<4; x++) {
				ret[i*4+j] += a[i*4+x] * b[x*4+j];
			}
		}
	}
}