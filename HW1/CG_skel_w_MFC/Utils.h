#pragma once

#include "mat.h"


typedef unsigned int UINT;

typedef struct Vertex {
	vec3 point;
	UINT vertex_index;
	UINT face_index;

	Vertex() : point(0, 0, 0), vertex_index(0), face_index(0) {}
	Vertex(vec3 a, UINT v_indx, UINT f_indx) { point = a; vertex_index = v_indx; face_index = f_indx; }
	~Vertex() {}
} Vertex;

typedef struct Line {
	double slope;
	double y_intercept;
	Line() = delete;
	Line(vec2 a, vec2 b)
	{
		slope = (b.y - a.y) / (b.x - a.x);
		y_intercept = (a.y - slope * a.x);
	}
};