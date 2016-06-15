#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <cstdarg>
#include <cmath>
#include <algorithm>

#include "globals.h"
#include "utils.h"

using std::sqrt;
using std::pow;
using std::min;
using std::max;

class MapTile;
class GLMapTile;

struct Point {
	GLfloat x,y,z,w;
	
	Point(GLfloat x=0.0f, GLfloat y=0.0f, GLfloat z=0.0f) : x(x), y(y), z(z), w(0) {}
	
	Point operator+(const Point b) {
		return Point(x+b.x, y+b.y, z+b.z);
	}
	
	Point operator+=(const Point b) {
		x+=b.x;
		y+=b.y;
		z+=b.z;
		return *this;
	}
	
	Point operator-=(const Point b) {
		x-=b.x;
		y-=b.y;
		z-=b.z;
		return *this;
	}
	
	Point operator*=(const GLfloat f) {
		x*=f;
		y*=f;
		z*=f;
		return *this;
	}
	
	Point operator-(const Point b) {
		return Point(x-b.x, y-b.y, z-b.z);
	}
	
	Point operator/(const Point b) {
		return Point(x/b.x, y/b.y, z/b.z);
	}
	
	Point operator-() {
		return Point(-x, -y, -z);
	}
	
	Point operator*(const GLfloat f) {
		return Point(f*x, f*y, f*z);
	}
	
	Point operator/(GLfloat b) {
		return Point(x/b, y/b, z/b);
	}
	
	Point cross(Point b) {
		Point ret;
		
		ret.x = y*b.z - z*b.y;
		ret.y = z*b.x - x*b.z;
		ret.z = x*b.y - y*b.x;
		
		return ret;
	}
	
	GLfloat dot(Point b) {
		return x*b.x + y*b.y + z*b.z;
	}
	
	Point projectOnto(Point b) {
		return b*(dot(b) / b.length());
	}
	
	GLfloat length() {
		return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	}
	
	Point normalise(GLfloat toLen = 1.0f) {
		if(toLen == 0)
			return *this;
		
		GLfloat len = length()/toLen;
		
		if(len == 0)
			return *this;
		
		x /= len;
		y /= len;
		z /= len;
		
		return *this;
	}
	
	Point rotate(GLfloat rotation) {
		GLfloat s = sin(rotation);
		GLfloat c = cos(rotation);
		
		/*GLfloat rotation[9] = {
			c, 		s, 		0.0f,
			-s,		c, 		0.0f,
			0.0f, 	0.0f, 	1.0f
		};*/
		
		Point r;
		
		r.x = c*x + s*y;
		r.y = -s*x + c*x;
		
		x = r.x;
		y = r.y;
		
		return *this;
	}
	
	static Point random() {
		return Point(randomFloat(), randomFloat(), randomFloat());
	}
};

struct Pointi {
	GLint x,y,z;
	
	Pointi(GLint x=0, GLint y=0, GLint z=0) : x(x), y(y), z(z) {}
};

struct Point2D {
	GLfloat x,y;
	
	Point2D(GLfloat x=0, GLfloat y=0) : x(x), y(y) {}
};

struct Ray {
	Point a, b, dir;
	
	Ray(Point a, Point b, Point dir) : a(a), b(b), dir(dir) {}
	Ray() {}
};

struct Size {
	GLfloat w,h,d;
	
	Size(GLfloat width=0.0f, GLfloat height=0.0f, GLfloat depth=0.0f) : w(width), h(height), d(depth) {}
	
	bool operator!=(const Size &b) {
		return ((w == w) && (h == h) && (d == d));
	}
	
	Size operator+(const Size &b) {
		return Size(w+b.w, h+b.h, d+b.d);
	}
	
	Point toPoint() {
		return Point(w, h, d);
	}
};

struct Colour {
	GLfloat r,g,b,a;
		
	Colour(GLfloat r=1.0f, GLfloat g=1.0f, GLfloat b=1.0f, GLfloat a=1.0f) : r(r), g(g), b(b), a(a) {}
	
	void blend(Colour a, Colour b, Colour c, Colour d, Colour e, Colour f, Colour g, Colour h) {
		GLfloat sum_r,sum_g,sum_b,sum_a;
		
		sum_r = a.r + b.r + c.r + d.r + e.r + f.r + g.r + h.r + this->r;
		sum_g = a.g + b.g + c.g + d.g + e.g + f.g + g.g + h.g + this->g;
		sum_b = a.b + b.b + c.b + d.b + e.b + f.b + g.b + h.b + this->b;
		sum_a = a.a + b.a + c.a + d.a + e.a + f.a + g.a + h.a + this->a;
		
		this->r = sum_r/9.0f;
		this->g = sum_g/9.0f;
		this->b = sum_b/9.0f;
		this->a = sum_a/9.0f;
	}
	
	static Colour average(int n, ...) {
		va_list arguments;
		GLfloat sum_r=0,sum_g=0,sum_b=0,sum_a=0;
		Colour val;
		
		va_start(arguments, n);
		
		for(int i=0; i<n; i++) {
			val = va_arg(arguments, Colour);
			
			sum_r += val.r;
			sum_g += val.g;
			sum_b += val.b;
			sum_a += val.a;
		}
		
		va_end(arguments);
		
		Colour ret;
		
		ret.r = sum_r/((GLfloat)n);
		ret.g = sum_g/((GLfloat)n);
		ret.b = sum_b/((GLfloat)n);
		ret.a = sum_a/((GLfloat)n);
		
		return ret;
	}
};

struct Vertex {
	Point p;
	Point n;
	Colour c;
	Point2D t;
	
	static GLuint offset(int i) {
		GLuint ret = 0;
		switch(i) {
			case 3:
				ret += sizeof(Colour);
			case 2:
				ret += sizeof(Point);
			case 1:
				ret += sizeof(Point);
		}
		return ret;
	}
};

struct QuadColours {
	Colour a,b,c,d;
	
	QuadColours(Colour a, Colour b, Colour c, Colour d) : a(a), b(b), c(c), d(d) {}
	QuadColours() {}
};

struct QuadVectors {
	Point a,b,c,d;
	
	QuadVectors(Point a, Point b, Point c, Point d) : a(a), b(b), c(c), d(d) {}
	QuadVectors() {}
};

struct QuadInts {
	GLint a,b,c,d;
	
	QuadInts(GLint a=0, GLint b=0, GLint c=0, GLint d=0) : a(a), b(b), c(c), d(d) {}
	GLfloat middle() {
		return ((GLfloat)(min(min(min(a,b), c), d)+max(max(max(a,b), c), d)))/2.0f;
	}
};

struct VideoInfo {
	Size window_size;
	GLfloat ratio;
	GLfloat fovY, zNear, zFar;
	
	VideoInfo(Size window_size, GLfloat fovY, GLfloat zNear, GLfloat zFar) : window_size(window_size), fovY(fovY), zNear(zNear), zFar(zFar) {
		ratio = (GLfloat)window_size.w/(GLfloat)window_size.h;
	}
	
	void resize(Size window_size) {
		this->window_size = window_size;
		ratio = (GLfloat)window_size.w/(GLfloat)window_size.h;
	}
};

#endif