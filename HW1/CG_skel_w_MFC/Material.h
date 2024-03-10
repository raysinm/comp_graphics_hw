#pragma once
//#include "Scene.h"
//#include "mat.h"
#include "Utils.h"
static const vec3 whiteColor = vec3(1, 1, 1);

class Reflection
{

};

class SpecularReflection : public Reflection
{
};

class Material	// Uniform?
{
private:
	// Colors:
	vec3 c_emissive, c_diffuse, c_specular;

public:

	float Ka, Kd, Ks;
	float EmissiveFactor;
	int COS_ALPHA = DEFUALT_LIGHT_ALPHA;
	Material() : c_emissive(whiteColor), c_diffuse(whiteColor), c_specular(whiteColor)
	{
		Ka = Kd = Ks = DEFUALT_LIGHT_K_VALUE;
		EmissiveFactor = DEFUALT_EMIS_FACTOR;
	}
	Material(vec3 emis, vec3 diff, vec3 spec) : c_emissive(emis), c_diffuse(diff), c_specular(spec){}
	vec3& getEmissive() { return c_emissive; }
	vec3& getDiffuse() { return c_diffuse; }
	vec3& getSpecular() { return c_specular; }

};