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
	vec3 c_emissive, c_diffuse, c_specular;	// Emissive == ambient?
};