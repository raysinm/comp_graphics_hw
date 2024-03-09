#include "Poly.h"

Poly::Poly(vec3& a, vec3& b, vec3& c, vec3& va, vec3& vb, vec3& vc, vec3& faceNormal, Material* mate, int id,\
		   vec3& a_cameraspace, vec3& b_cameraspace, vec3& c_cameraspace)
{
	this->a   = a;
	this->b   = b;
	this->c   = c;
	this->vnA = va;
	this->vnB = vb;
	this->vnC = vc;
	this->fn  = faceNormal;
	this->material = mate;
	this->id = id;
	this->a_cameraspace = a_cameraspace;
	this->b_cameraspace = b_cameraspace;
	this->c_cameraspace = c_cameraspace;
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

}

UINT Poly::Depth(int x, int y)
{
	UINT Zp, Zi;
	double Ti, t;
	vec2 p1, p2, p3;
	UINT z1, z2, z3;
	vec2 Ps = vec2(x, y);

	Line mainLine = Line(vec2(a.x, a.y), Ps);
	Line baseLine = lines[LINE_BC];
	p1 = vec2(b.x, b.y); z1 = (UINT)b.z;
	p2 = vec2(c.x, c.y); z2 = (UINT)c.z;
	p3 = vec2(a.x, a.y); z3 = (UINT)a.z;

	if (mainLine.isParallel(this->lines[LINE_AC]))
	{
		mainLine = Line(vec2(b.x, b.y), Ps);
		baseLine = lines[LINE_AC];
		p1 = vec2(a.x, a.y); z1 = (UINT)a.z;
		p2 = vec2(c.x, c.y); z2 = (UINT)c.z;
		p3 = vec2(b.x, b.y); z3 = (UINT)b.z;
	}
	else if (mainLine.isParallel(this->lines[LINE_AB]))
	{
		mainLine = Line(vec2(c.x, c.y), Ps);
		baseLine = lines[LINE_AB];
		p1 = vec2(a.x, a.y); z1 = (UINT)a.z;
		p2 = vec2(b.x, b.y); z2 = (UINT)b.z;
		p3 = vec2(c.x, c.y); z3 = (UINT)c.z;
	}
	
	vec2 Pi;
	bool is_par = false;
	Pi = mainLine.intersect(baseLine, &is_par);
	if (is_par)
		return max_z;
	
	//Ti = abs(length(Pi - p1)) / abs(length(p2 -p1));
	// 
	//float dotProduct = glm::dot(pMinusP0, p1MinusP0);
	//float lengthSquared = glm::dot(p1MinusP0, p1MinusP0);

	//float t = dotProduct / lengthSquared;

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

vec3& Poly::getPoint(int index)
{
	switch (index)
	{
	case 0:
		return a;
		break;
	case 1:
		return b;
		break;
	case 2:
		return c;
		break;
	}

}
