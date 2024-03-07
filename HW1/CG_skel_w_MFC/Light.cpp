#include "Light.h"

Light::Light(vec3 pos, vec3 dir, LIGHT_TYPE ltype)
{
	_position = pos;
	_direction = normalize(dir);
	_type = ltype;
}
