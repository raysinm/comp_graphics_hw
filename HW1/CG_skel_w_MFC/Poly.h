#pragma once
#include "mat.h"
#include "Scene.h"


using namespace std;

class Poly
{
private:
	vec3 a, b, c;
	vec3 vnA, vnB, vnC;
	vec3 fn;

	vec3 min_y_vertex;
	vec3 max_y_vertex;
public:
	Poly() {};
	Poly(vec3& a, vec3& b, vec3& c, vec3& va, vec3& vb, vec3& vc, vec3& faceNormal);
	~Poly(void) {};
};
