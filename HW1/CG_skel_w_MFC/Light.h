#pragma once
#include "mat.h"
#include "Utils.h"

using namespace std;

static const vec3 down = vec3(0, -1, 0);
class Light
{
private:
	vec3 _position, _direction;	// World space
	vec3 _color;

	vec4 _positionCameraSpace;
	
	LIGHT_TYPE _type;
	string _name;

public:
	float La, Ld, Ls;
	Light(vec3 pos = vec3(0), vec3 dir = down, LIGHT_TYPE ltype = AMBIENT_LIGHT);
	~Light() {}

	bool selected = false;

	void setPosition(vec3& trnsl) { _position = trnsl; }
	void setDirection(vec3& dir) { _direction = dir; }
	void setName(string name) { _name = name; }
	void setColor(vec3& col) { _color = col; }
	void setLightType(int t) { _type = (LIGHT_TYPE) t; }
	void resetPosition() { _position = vec3(0, 0, 0); }
	void resetDirection() { _direction = down; }

	vec3 getPosition() { return _position; }
	vec3 getDirection() { return -normalize(_direction); }
	vec3* getDirectionPtr() { return &_direction; }
	vec3* getPositionPtr() { return &_position; }
	vec3& getColor() { return _color; }

	string getName() { return _name; }
	LIGHT_TYPE getLightType() { return _type; }
};