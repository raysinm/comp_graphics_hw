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
	vector<Material> material;
	Material userDefinedMate;
	vec3 fn;
	vec3 centerOfPoly;

	int min_y;
	int max_y;
	int min_x;
	int max_x;
	UINT min_z;
	UINT max_z;

	std::vector<Line> lines;
	//Helper Function:
	vector<float> getBarycentricCoords(vec2& pixl);
	vector<float> getBarycentricCoords(vec3& pos);
	Material helper_result;
public:
	int id;
	bool FLAT_calculatedColor = false;
	vec3 FLAT_calculatedColorValue;

	bool GOUROD_calculatedColors = false;
	vec3 GOUROD_colors[3];

	bool isUniformMaterial = true;

	~Poly(void) {};
	Poly(){};
	Poly(	vec3& a, vec3& b, vec3& c, vec3& va, vec3& vb, vec3& vc, vec3& faceNormal, bool isUniform, vector<Material>& mates,\
			Material& uniformMaterial, int vertInd1, int vertInd2, int vertInd3,\
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
	vec3 GOUROD_interpolate(vec2& pixl);
	vec3& GetPoint(int index) { return points_cameraspace[index]; }
	vec3& GetVN(int index) { return vn[index]; }
	Material& GetMaterial(int index) { return material[index]; }
	Material& InterpolateMaterial(vec2& pixl);
	Material& InterpolateMaterial(vec3& pos);
	vec3 PHONG_interpolatePosition(vec2& pixl);
	vec3 PHONG_interpolateNormal(vec2& pixl);
};
