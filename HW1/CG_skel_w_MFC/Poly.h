#pragma once
//#include "mat.h"
#include "Material.h"

using namespace std;

class Poly
{

	// Should contain screen coordinates only (pixel positions):
	// 
	// a. Should contain a set of three 'original' screen coordinates, normalized in relation to screen size even if they are outside of the screen 
	//	  (for color calculations)
	// 
	// b. MAYBE (perhaps will be claculated during scanline):
	//	  Should contain an additional vector<vec3> of actual screen coordinates- could contain a maximum
	//	  of 6 coords in case that all triangle vertices are outside of the screen borders (and a minimum of 3
	//	  in case all are inside the screen).
	//	  This will be used for scanline calculations only- can be seen as "the part of the triangle that is inside
	//	  the screen and is not necesseraly a triangle anymore"

private:
	vec3 a, b, c;
	//vector<vec3> screen_coords;

	vec2 l_AB, l_BC, l_AC;
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
