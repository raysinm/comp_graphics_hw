#pragma once
//#include "Scene.h"
//#include "mat.h"
#include "Utils.h"

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
	int COS_ALPHA = 1;
	Material() : c_emissive(1,1,1), c_diffuse(1, 1, 1), c_specular(1, 1, 1)
	{
		Ka = Kd = Ks = EmissiveFactor = 0.5f;
	}
	Material(vec3 emis, vec3 diff, vec3 spec) : c_emissive(emis), c_diffuse(diff), c_specular(spec){}
	vec3& getEmissive() { return c_emissive; }
	vec3& getDiffuse() { return c_diffuse; }
	vec3& getSpecular() { return c_specular; }

};