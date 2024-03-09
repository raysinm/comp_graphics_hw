#pragma once

#include "mat.h"
#include <stdexcept>

typedef unsigned int UINT;
//#define INDEX(width,x,y,c) (x+y*width)*3 + c
//#define Z_INDEX(width,x,y) (x+(y*width))
#define RED   0
#define GREEN 1
#define BLUE  2
#define MAX_Z 65535
#define LIGHT_DEFAULT_NAME "Light"

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
	bool operator==(Line& other) { return (this->_slope == other._slope && this->_b == other._b); }

	vec2 intersect(Line& other, bool* isParallel)
	{
		/* We assume that *this line is a scanline (slope = 0) */
		/* Handle Vertical line */
		if (other.getIsVertical())
		{
			if (this->isVertical)
			{
				//special case - both vertical
				*isParallel = true;
				if (this->verticalX == other.verticalX)
					return vec2(verticalX, 0);
				else
				{
					return vec2(0, 0);
				}
			}
			else
			{
				return vec2(other.getVerticalX(), this->y(other.getVerticalX()));

			}
		}

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
