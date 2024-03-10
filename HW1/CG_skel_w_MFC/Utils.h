#pragma once

#include "mat.h"
#include <stdexcept>

typedef unsigned int UINT;
#define RED   0
#define GREEN 1
#define BLUE  2
#define MAX_Z 65535
#define LIGHT_DEFAULT_NAME "Light"
#define EPSILON 0.00001
#define DEFUALT_EMIS_FACTOR 0.15
#define DEFUALT_LIGHT_K_VALUE 0.5
#define DEFUALT_LIGHT_ALPHA 1




inline int Index(int width, int x, int y, int c) { return (x + y * width) * 3 + c; }
inline int Z_Index(int width, int x, int y) { return x + (y * width); }

enum DrawAlgo {
	WIRE_FRAME,
	FLAT,
	GOURAUD,
	PHONG,
	COUNT
};

typedef struct Vertex {
	vec3 point;
	vec3 point_cameraspace;
	UINT vertex_index;
	UINT face_index;

	Vertex() : point(0, 0, 0), vertex_index(0), face_index(0) {}
	Vertex(vec3& a, UINT v_indx, UINT f_indx, vec3& a_cameraspace) { point = a; vertex_index = v_indx; face_index = f_indx; point_cameraspace = a_cameraspace; }
	~Vertex() {}
} Vertex;

class Line {
private:
	double _slope;
	double _b;

	bool isVertical=false;
	double verticalX =0;


	vec2 a, b;
public:
	Line() : _slope(1), _b(0){}
	Line(double slope, double b) : _slope(slope), _b(b){}
	Line(vec2& a, vec2& b)
	{
		this->a = a;
		this->b = b;

		isVertical = (abs(a.x - b.x) < EPSILON);

		if (!isVertical) {
			_slope = (b.y - a.y) / (b.x - a.x);
			_b = (a.y - _slope * a.x);
		}
		else
		{
			verticalX = a.x;
		}
	}
	bool operator==(Line& other)
	{
		return ( abs(this->_slope - other._slope) < EPSILON &&\
				 abs(this->_b - other._b) < EPSILON);
	}

	vec2 intersect(Line& other, bool* isParallel)
	{
		/* We assume that *this line is a scanline (slope = 0) */
		/* Handle Vertical line */
		if (other.getIsVertical())
		{
			if (this->isVertical)
			{
				*isParallel = true;
				if (abs(this->verticalX - other.verticalX) < EPSILON)
					return vec2(verticalX, 0);
				else
					return vec2(0, 0);
			}
			else
				return vec2(other.getVerticalX(), this->y(other.getVerticalX()));
		}
		if (this->isVertical)
		{
			if (other.getIsVertical())
			{
				*isParallel = true;
				if (abs(this->verticalX - other.verticalX) < EPSILON)
					return vec2(verticalX, 0);
				else
					return vec2(0, 0);
			}
			else
				return vec2(this->verticalX, other.y(this->verticalX));
		}


		/* Handle Parallel lines */
		if (abs(this->_slope - other._slope) < EPSILON)
		{
			//throw ParallelLinesException("Parallel Lines");
			*isParallel = true;
			return vec2(0);
		}
		else
			*isParallel = false;
		/* Handle all other lines */
		double x = (other._b - this->_b) / (this->_slope - other._slope);
		double y = this->y(x);

		return vec2(x, y);
	}
	double y(double x) { 
		return (_slope * x + _b);
	}

	bool getIsVertical() { return isVertical; }
	double getVerticalX() { return verticalX; }
	double getSlope() { return _slope; }
	vec2& getA() { return a; }
	vec2& getB() { return b; }
	bool isParallel(Line& other)
	{
		if (isVertical)
			return other.getIsVertical();

		return abs(_slope - other.getSlope()) < EPSILON;
	}
};


typedef enum LIGHT_TYPE
{
	AMBIENT_LIGHT	= 0,
	POINT_LIGHT		= 1,
	PARALLEL_LIGHT	= 2
}LIGHT_TYPE;

