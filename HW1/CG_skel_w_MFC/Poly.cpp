#include "Poly.h"

Poly::Poly(vec3& a, vec3& b, vec3& c, vec3& va, vec3& vb, vec3& vc, vec3& faceNormal)
{
	this->a   = a;
	this->b   = b;
	this->c   = c;
	this->vnA = va;
	this->vnB = vb;
	this->vnC = vc;
	this->fn  = faceNormal;

	// Calculate y min, y max of polygon NOTICE: could be outside of screen
	min_y = (int) min(min(a.y, b.y), c.y);
	max_y = (int) max(max(a.y, b.y), c.y);
			 
	min_x = (int)min(min(a.x, b.x), c.x);
	max_x = (int)max(max(a.x, b.x), c.x);

	min_z = (int)min(min(a.z, b.z), c.z);

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
		return min_z;
	//try
	//{
	//	Pi = mainLine.intersect(baseLine);
	//}
	//catch (const std::exception&)
	//{
	//	//There is some edge cases where the 2 line could be parallel...
	//	// Even there is some edge cases that the lines are the same lines !
	//	// This is the case where the polygon plane, is excactly 90 degress to the camera.
	//	// So the camera only sees a line instead of a triangle...
	//	return this->GetMinZ();
	//}
	
	Ti = abs(length(Pi - p1)) / abs(length(p2 -p1));
	Zi = (UINT)((Ti * z2) + (1 - Ti) * z1);
 
	t = abs(length(Ps - p3)) / abs(length(Pi - p3));
	Zp = (UINT)(t * Zi + (1 - t) * z3);

	return Zp;
}
