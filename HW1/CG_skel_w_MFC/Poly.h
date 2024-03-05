#pragma once
//#include "mat.h"
#include "Material.h"
#include "Utils.h"
#include <vector>

using namespace std;

#define LINE_AB 0
#define LINE_AC 1
#define LINE_BC 2

class Poly
{
	// Should contain screen coordinates only (pixel positions):
	// a. Should contain a set of three 'original' screen coordinates, normalized in relation to screen size even if they are outside of the screen 
	//	  (for color calculations)

private:
	vec3 a, b, c;
	vec3 vnA, vnB, vnC;
	vec3 fn;

	int min_y;
	int max_y;
	int min_x;
	int max_x;
	UINT min_z;

	std::vector<Line> lines;
public:

	Poly(){};
	Poly(vec3& a, vec3& b, vec3& c, vec3& va, vec3& vb, vec3& vc, vec3& faceNormal);

	//GetColor(Material& material, DrawAlgo& draw_algo);	//Will be used to calculate color?
	~Poly(void) {};

	int GetMinY() { return min_y; }
	int GetMaxY() { return max_y; }
	int GetMinX() { return min_x; }
	int GetMaxX() { return max_x; }
	int GetMinZ() { return min_z; }

	std::vector<Line>& GetLines() { return lines; }
	UINT Depth(int x, int y);

};
