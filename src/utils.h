#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <vector>

#include "globals.h"

GLint averagei(int n, ...);

std::vector<std::string> split(const std::string &s, char delim);

extern int ids;
int getNewId();
std::string readFile(const char *filename);
GLfloat randomFloat();
void matrixMultiply44(GLfloat a[16], GLfloat b[16], GLfloat ret[16]);

#endif