#pragma once
#include "Utils.h"

static const vec3 whiteColor = vec3(1, 1, 1);

class Material
{

public:
	// Colors:
	vec3 c_emissive, c_diffuse, c_specular;
	float Ka, Kd, Ks;
	float EmissiveFactor;
	int COS_ALPHA = DEFUALT_LIGHT_ALPHA;
	Material()
	{
		c_emissive = whiteColor;
		c_diffuse = whiteColor;
		c_specular = whiteColor;
		Ka = DEFUALT_LIGHT_Ka_VALUE;
		Kd = DEFUALT_LIGHT_Kd_VALUE;
		Ks = DEFUALT_LIGHT_Ks_VALUE;
		EmissiveFactor = DEFUALT_EMIS_FACTOR;
		COS_ALPHA = DEFUALT_LIGHT_ALPHA;
	}
	Material(vec3 emis, vec3 diff, vec3 spec) : c_emissive(emis), c_diffuse(diff), c_specular(spec){}
	Material(vec3 emis, vec3 diff, vec3 spec, float ka, float kd, float ks, float emisFactor, int alpha) :
			c_emissive(emis), c_diffuse(diff), c_specular(spec), Ka(ka), Kd(kd), Ks(ks),\
			EmissiveFactor(emisFactor), COS_ALPHA(alpha) {}
	vec3& getEmissive() { return c_emissive; }
	vec3& getDiffuse() { return c_diffuse; }
	vec3& getSpecular() { return c_specular; }


	Material operator * (const GLfloat s) const
	{
		Material res = Material (c_emissive, c_diffuse, c_specular, Ka, Kd, Ks, EmissiveFactor, COS_ALPHA);

		res.c_emissive *= s;
		res.c_diffuse *= s;
		res.c_specular *= s;
		res.Ka *= s;
		res.Kd *= s;
		res.Ks *= s;
		res.EmissiveFactor *= s;
		res.COS_ALPHA *= s;

		return res;
	}

	friend Material operator * (const GLfloat s, const Material& v)
	{
		return v * s;
	}

	Material& operator += (const Material& v)
	{
		c_emissive += v.c_emissive;
		c_diffuse  += v.c_diffuse;
		c_specular += v.c_specular;
		Ka += v.Ka;
		Kd += v.Kd;
		Ks += v.Ks;
		EmissiveFactor += v.EmissiveFactor;
		COS_ALPHA += v.COS_ALPHA;

		return *this;
	}
};