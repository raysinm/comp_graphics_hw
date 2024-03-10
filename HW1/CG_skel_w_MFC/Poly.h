#pragma once
#include "Material.h"
#include "Utils.h"
#include <vector>

using namespace std;

#define LINE_AB 0
#define LINE_AC 1
#define LINE_BC 2

class Poly
{
private:
	vec3 points[3];
	vec3 points_cameraspace[3];
	vec3 vn[3];
	vec3 fn;
	vec3 centerOfPoly;

	int min_y;
	int max_y;
	int min_x;
	int max_x;
	UINT min_z;
	UINT max_z;

	std::vector<Line> lines;
public:
	int id;
	bool FLAT_calculatedColor = false;
	vec3 FLAT_calculatedColorValue;

	bool GOUROD_calculatedColors = false;
	vec3 GOUROD_colors[3];

	~Poly(void) {};
	Poly(){};
	Poly(vec3& a, vec3& b, vec3& c, vec3& va, vec3& vb, vec3& vc, vec3& faceNormal, Material* mate, int id,\
		 vec3& a_cameraspace, vec3& b_cameraspace, vec3& c_cameraspace);


	int GetMinY() { return min_y; }
	int GetMaxY() { return max_y; }
	int GetMinX() { return min_x; }
	int GetMaxX() { return max_x; }
	int GetMinZ() { return min_z; }

	vec3 GetFaceNormal() { return vec3(fn.x, fn.y, fn.z); }
	vec3& GetCenter() { return centerOfPoly; }
	std::vector<Line>& GetLines() { return lines; }
	UINT Depth(int x, int y);
	Material* material;
	vec3 GOUROD_interpolate(vec2& pixl);
	vec3& GetPoint(int index) { return points_cameraspace[index]; }
	vec3& GetVN(int index) { return vn[index]; }
};
