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

	min_y = (UINT) min(min(a.y, b.y), c.y);
	max_y = (UINT) max(max(a.y, b.y), c.y);
}