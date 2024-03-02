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
	min_y = (UINT) min(min(a.y, b.y), c.y);
	max_y = (UINT) max(max(a.y, b.y), c.y);
	
	min_y = (UINT)min(min(a.x, b.x), c.x);
	max_y = (UINT)max(max(a.x, b.x), c.x);

	// Set triangle lines
	lines.push_back(Line(vec2(a.x, a.y), vec2(b.x, b.y)));
	lines.push_back(Line(vec2(b.x, b.y), vec2(c.x, c.y)));
	lines.push_back(Line(vec2(a.x, a.y), vec2(c.x, c.y)));

}