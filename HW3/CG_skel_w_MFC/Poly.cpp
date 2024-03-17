#include "Poly.h"

Poly::Poly(	vec3& a, vec3& b, vec3& c, vec3& va, vec3& vb, vec3& vc, vec3& faceNormal, bool isUniform, vector<Material>& mates, \
			Material& uniformMaterial, int vertInd1, int vertInd2, int vertInd3, \
			vec3& a_cameraspace, vec3& b_cameraspace, vec3& c_cameraspace)
{
	this->points[0]   = a;
	this->points[1]   = b;
	this->points[2]   = c;
	this->vn[0] = va;
	this->vn[1] = vb;
	this->vn[2] = vc;
	this->fn  = faceNormal;
	this->isUniformMaterial = isUniform;
	this->points_cameraspace[0] = a_cameraspace;
	this->points_cameraspace[1] = b_cameraspace;
	this->points_cameraspace[2] = c_cameraspace;
	this->centerOfPoly = (a_cameraspace + b_cameraspace + c_cameraspace) / 3.0f;

	// Calculate y min, y max of polygon NOTICE: could be outside of screen
	min_y = (int) min(min(a.y, b.y), c.y);
	max_y = (int) max(max(a.y, b.y), c.y);
			 
	min_x = (int)min(min(a.x, b.x), c.x);
	max_x = (int)max(max(a.x, b.x), c.x);

	min_z = (int)min(min(a.z, b.z), c.z);
	max_z = (int)max(max(a.z, b.z), c.z);
	
	// Set triangle lines
	lines.push_back(Line(vec2(a.x, a.y), vec2(b.x, b.y)));
	lines.push_back(Line(vec2(a.x, a.y), vec2(c.x, c.y)));
	lines.push_back(Line(vec2(b.x, b.y), vec2(c.x, c.y)));

	if (isUniform)
		this->userDefinedMate = uniformMaterial;
	else
	{
		this->material = vector<Material>(3);
		this->material[0] = mates[vertInd1];
		this->material[1] = mates[vertInd2];
		this->material[2] = mates[vertInd3];
	}
	helper_result = Material();
}

vector<float> Poly::getBarycentricCoords(vec2& pixl)
{
	vec2 p1 = vec2(points[0].x, points[0].y);
	vec2 p2 = vec2(points[1].x, points[1].y);
	vec2 p3 = vec2(points[2].x, points[2].y);


	vec3 p = vec3(pixl, 0);
	vec3 p1p = vec3(p1, 0) - p;
	vec3 p2p = vec3(p2, 0) - p;
	vec3 p3p = vec3(p3, 0) - p;

	float Ai[3] = { 0 };

	Ai[0] = length(cross(p3p, p2p)) / 2.0;
	Ai[1] = length(cross(p1p, p3p)) / 2.0;
	Ai[2] = length(cross(p1p, p2p)) / 2.0;

	float sum = Ai[0] + Ai[1] + Ai[2];

	vector<float> alpha(3);
	for (int i = 0; i < 3; i++) {
		alpha[i] = Ai[i] / sum;
	}

	return alpha;
}

vector<float> Poly::getBarycentricCoords(vec3& pos)
{
	vec3 p1 = points_cameraspace[0];
	vec3 p2 = points_cameraspace[1];
	vec3 p3 = points_cameraspace[2];


	vec3 p = pos;
	vec3 p1p = p1 - p;
	vec3 p2p = p2 - p;
	vec3 p3p = p3 - p;

	float Ai[3] = { 0 };

	Ai[0] = length(cross(p3p, p2p)) / 2.0;
	Ai[1] = length(cross(p1p, p3p)) / 2.0;
	Ai[2] = length(cross(p1p, p2p)) / 2.0;

	float sum = Ai[0] + Ai[1] + Ai[2];

	vector<float> alpha(3);
	for (int i = 0; i < 3; i++) {
		alpha[i] = Ai[i] / sum;
	}

	return alpha;
}

UINT Poly::Depth(int x, int y)
{
	UINT Zp, Zi;
	double Ti, t;
	vec2 p1, p2, p3;
	UINT z1, z2, z3;
	vec2 Ps = vec2(x, y);

	Line mainLine = Line(vec2(points[0].x, points[0].y), Ps);
	Line baseLine = lines[LINE_BC];
	p1 = vec2(points[1].x, points[1].y); z1 = (UINT)points[1].z;
	p2 = vec2(points[2].x, points[2].y); z2 = (UINT)points[2].z;
	p3 = vec2(points[0].x, points[0].y); z3 = (UINT)points[0].z;

	if (mainLine.isParallel(this->lines[LINE_AC]))
	{
		mainLine = Line(vec2(points[1].x, points[1].y), Ps);
		baseLine = lines[LINE_AC];
		p1 = vec2(points[0].x, points[0].y); z1 = (UINT)points[0].z;
		p2 = vec2(points[2].x, points[2].y); z2 = (UINT)points[2].z;
		p3 = vec2(points[1].x, points[1].y); z3 = (UINT)points[1].z;
	}
	else if (mainLine.isParallel(this->lines[LINE_AB]))
	{
		mainLine = Line(vec2(points[2].x, points[2].y), Ps);
		baseLine = lines[LINE_AB];
		p1 = vec2(points[0].x, points[0].y); z1 = (UINT)points[0].z;
		p2 = vec2(points[1].x, points[1].y); z2 = (UINT)points[1].z;
		p3 = vec2(points[2].x, points[2].y); z3 = (UINT)points[2].z;
	}
	
	vec2 Pi;
	bool is_par = false;
	Pi = mainLine.intersect(baseLine, &is_par);
	if (is_par)
		return max_z;

	vec2 piMINUSp1 = Pi - p1;
	vec2 p2MINUSp1 = p2 - p1;

	Ti = length(piMINUSp1) / length(p2MINUSp1);
	Zi = (UINT) (round((Ti * z2) + (1 - Ti) * z1));
 

	vec2 psMINUSp3 = Ps - p3;
	vec2 piMINUSp3 = Pi - p3;

	t = length(psMINUSp3)/ length(piMINUSp3);
	//t = abs(length(Ps - p3)) / abs(length(Pi - p3));
	Zp = (UINT) (round(t * Zi + (1 - t) * z3));

	return Zp;
}

vec3 Poly::GOUROD_interpolate(vec2& pixl)
{
	vector<float> alpha = getBarycentricCoords(pixl);

	vec3 result(0);
	for (int i = 0; i < 3; i++)
		result += alpha[i] * GOUROD_colors[i];
	
	return result;
}

Material& Poly::InterpolateMaterial(vec2& pixl)
{
	if (isUniformMaterial)
		return material[0];

	vector<float> alpha = getBarycentricCoords(pixl);

	helper_result = Material();
	for (int i = 0; i < 3; i++)
		helper_result += alpha[i] * material[i];

	return helper_result;

}

Material& Poly::InterpolateMaterial(vec3& pos)
{
	if (isUniformMaterial)
		return userDefinedMate;

	vector<float> alpha = getBarycentricCoords(pos);

	helper_result = Material() * 0;
	for (int i = 0; i < 3; i++)
		helper_result += alpha[i] * material[i];

	return helper_result;
}

vec3 Poly::PHONG_interpolatePosition(vec2& pixl)
{
	vector<float> alpha = getBarycentricCoords(pixl);

	vec3 result(0);
	for (int i = 0; i < 3; i++)
		result += alpha[i] * points_cameraspace[i];

	return result;
}

vec3 Poly::PHONG_interpolateNormal(vec2& pixl)
{
	vector<float> alpha = getBarycentricCoords(pixl);

	vec3 result(0);
	for (int i = 0; i < 3; i++)
		result += alpha[i] * vn[i];

	return result;
}