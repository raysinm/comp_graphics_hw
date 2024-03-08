#include "Light.h"

Light::Light(vec3 pos, vec3 dir, LIGHT_TYPE ltype)
{
	_position = pos;
	_direction = normalize(dir);
	_type = ltype;
	_color = (1, 1, 1);
	_name = string(LIGHT_DEFAULT_NAME);
	La = Ld = Ls = 1;
}
