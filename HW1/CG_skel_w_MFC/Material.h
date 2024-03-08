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
	Material() : c_emissive(0.5,0.5,0.5), c_diffuse(0.5, 0.5, 0.5), c_specular(0.5, 0.5, 0.5)
	{
		Ka = Kd = Ks = 1;
	}
	Material(vec3 emis, vec3 diff, vec3 spec) : c_emissive(emis), c_diffuse(diff), c_specular(spec){}
	vec3& getEmissive() { return c_emissive; }
	vec3& getDiffuse() { return c_diffuse; }
	vec3& getSpecular() { return c_specular; }

};