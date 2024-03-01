#pragma once
#include "mat.h"
#include "Material.h"

using namespace std;

class Poly
{
private:
	vec3 a, b, c;
	vec3 vnA, vnB, vnC;
	vec3 fn;

	UINT min_y;
	UINT max_y;

public:
	Poly() {};
	Poly(vec3& a, vec3& b, vec3& c, vec3& va, vec3& vb, vec3& vc, vec3& faceNormal);
	//GetColor(Material& material, DrawAlgo& draw_algo);	//Will be used to calculate color?
	~Poly(void) {};

	UINT GetMin() { return min_y; }
	UINT GetMax() { return max_y; }
};
