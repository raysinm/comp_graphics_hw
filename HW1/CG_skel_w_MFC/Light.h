#pragma once
#include "mat.h"
#include "Utils.h"

using namespace std;


class Light
{
private:
	vec3 _position, _direction;	// World space
	vec3 _color;
	
	LIGHT_TYPE _type;
	string _name;

public:
	Light(vec3 pos = vec3(0), vec3 dir = vec3(0, -1, 0), LIGHT_TYPE ltype = AMBIENT_LIGHT);
	~Light() {}

	void setPosition(vec3& trnsl) { _position = trnsl; }
	void setDirection(vec3& dir)  { _direction = normalize(dir); }
	void setName(string name) { _name = name; }
	void setColor(vec3& col) { _color = col; }
};