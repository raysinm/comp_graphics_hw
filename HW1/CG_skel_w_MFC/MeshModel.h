#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

#define FINDEX(face) face*3

using namespace std;

class MeshModel : public Model
{
protected :
	MeshModel() {}
	vec3* vertex_positions;		// In model space 
	vec3* t_vertex_positions;	// Transformed	- Used for pipeline
	vec3* vertex_normals;
	vec2* buffer2d;				//Use this buffer to send the renederer for Rasterazation process.   Initiate once, update each frame.

	//mat4 _world_transform;	// Maybe later
	vec3 _t_positions;
	vec3 _r_degrees;	// in x, in y , in z 
	vec3 _s_scales;
	mat3 _normal_transform;

public:
	vec2* Get2dBuffer();
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void MeshModel::draw(mat4& cTransform);
};
