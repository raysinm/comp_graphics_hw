#pragma once

#include "mat.h"
#include <stdexcept>

typedef unsigned int UINT;

enum DrawAlgo {
	WIRE_FRAME,
	FLAT,
	GOURAUD,
	PHONG,
	COUNT
};

class LineException : public std::exception {
	std::string _message;
public:
	LineException(const std::string& message) 
		: std::exception(), _message(message) {}
};

class ParallelLinesException : public LineException {
public:
	ParallelLinesException(const std::string& message)
		: LineException(message) {}
};

class SlopeZeroException : public LineException {
public:
	SlopeZeroException(const std::string& message)
		: LineException(message) {}
};



typedef struct Vertex {
	vec3 point;
	UINT vertex_index;
	UINT face_index;

	Vertex() : point(0, 0, 0), vertex_index(0), face_index(0) {}
	Vertex(vec3 a, UINT v_indx, UINT f_indx) { point = a; vertex_index = v_indx; face_index = f_indx; }
	~Vertex() {}
} Vertex;

typedef class Line {
private:
	double _slope;
	double _b;

public:
	Line() : _slope(1), _b(0){}
	Line(double slope, double b) : _slope(slope), _b(b){}
	Line(vec2 a, vec2 b)
	{
		_slope = (b.y - a.y) / (b.x - a.x);
		_b = (a.y - _slope * a.x);
	}
	vec2 intersect(Line& other)
	{
		if ((this->_slope - other._slope) == 0)
		{
			// Lines are parallel
			throw ParallelLinesException("Parallel Lines");
		}
		double x, y;

		x = (other._b - this->_b) / (this->_slope - other._slope);
		y = this->y(x);

		return vec2(x, y);
	}
	double y(double x) { 
		return _slope * x + _b; 
	}
	double x(double y) { 
		if (_slope == 0)
			throw SlopeZeroException("No x solution, slope is zero");
		return (y - _b) / _slope; 
	}

}Line;
