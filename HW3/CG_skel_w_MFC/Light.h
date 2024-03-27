#pragma once
#include "mat.h"
#include "Utils.h"

using namespace std;

static const vec3 down = vec3(0, -1, 0);
class Light
{
private:
	vec3 _position, _direction;	// World space
	vec3 _color = vec3(1,1,1);

	vec4 _positionCameraSpace;
	vec3 _directionCameraSpace;
	
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
	void updatePosCameraSpace(mat4 cTransform) {
		_positionCameraSpace = cTransform * vec4(_position);
	}
	void updateDirCameraSpace(mat4 cTransform) {
		vec4 startPoint = cTransform * vec4(0, 0, 0, 1);
		startPoint /= startPoint.w;

		vec4 endPoint = cTransform * vec4(_direction);
		endPoint /= endPoint.w;

		vec4 res = endPoint - startPoint;
		_directionCameraSpace = vec3(res.x, res.y, res.z);
	}

	vec3 getPosition() { return _position; }
	vec3 getPositionCameraSpace() { return vec3(_positionCameraSpace.x, _positionCameraSpace.y, _positionCameraSpace.z) / _positionCameraSpace.w; }
	vec3 getDirection() { return -normalize(_direction); } /* In World space*/
	vec3 getDirectionCameraSpace() { return -normalize(_directionCameraSpace); }
	vec3* getDirectionPtr() { return &_direction; }
	vec3* getPositionPtr() { return &_position; }
	vec3& getColor() { return _color; }

	string getName() { return _name; }
	LIGHT_TYPE getLightType() { return _type; }
};