#pragma once

#include "mat.h"
#include <stdexcept>

typedef unsigned int UINT;
#define INDEX(width,x,y,c) (x+y*width)*3 + c
#define Z_INDEX(width,x,y) (x+(y*width))
#define RED   0
#define GREEN 1
#define BLUE  2
#define MAX_Z 65535
#define LIGHT_DEFAULT_NAME "Light"

enum DrawAlgo {
	WIRE_FRAME,
	FLAT,
	GOURAUD,
	PHONG,
	COUNT
};

typedef struct Vertex {
	vec3 point;
	UINT vertex_index;
	UINT face_index;

	Vertex() : point(0, 0, 0), vertex_index(0), face_index(0) {}
	Vertex(vec3 a, UINT v_indx, UINT f_indx) { point = a; vertex_index = v_indx; face_index = f_indx; }
	~Vertex() {}
} Vertex;

class Line {
private:
	double _slope;
	double _b;

	bool isVertical;
	double verticalX;

public:
	Line() : _slope(1), _b(0){}
	Line(double slope, double b) : _slope(slope), _b(b){}
	Line(vec2 a, vec2 b)
	{
		isVertical = (a.x == b.x);

		if (!isVertical) {
			_slope = (b.y - a.y) / (b.x - a.x);
			_b = (a.y - _slope * a.x);
		}
		else
		{
			verticalX = a.x;
		}
	}
	vec2 intersect(Line& other, bool* isParallel)
	{
		/* We assume that *this line is a scanline (slope = 0) */
		/* Handle Vertical line */
		if (other.getIsVertical())
			return vec2(other.getVerticalX(), _b);

		/* Handle Parallel lines */
		if ((this->_slope - other._slope) == 0)
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
		return _slope * x + _b; 
	}

	bool getIsVertical() { return isVertical; }
	double getVerticalX() { return verticalX; }
	double getSlope() { return _slope; }

	bool isParallel(Line& other)
	{
		if (isVertical)
			return other.getIsVertical();

		return _slope == other.getSlope();
	}
};


typedef enum LIGHT_TYPE
{
	AMBIENT_LIGHT	= 0,
	POINT_LIGHT		= 1,
	PARALLEL_LIGHT	= 2
}LIGHT_TYPE;
