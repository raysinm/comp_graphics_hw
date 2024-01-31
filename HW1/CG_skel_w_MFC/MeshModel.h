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
	vec3* vertex_positions;	// In model space 
	vec3* t_vertex_positions;	// Transformed	- Used for pipeline	- No z axis
	vec3* vertex_normals;

	//mat4 _world_transform;	// Maybe later
	vec3 _t_positions;
	vec3 _r_degrees;	// in x, in y , in z 
	vec3 _s_scales;
	mat3 _normal_transform;

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void MeshModel::draw(mat4& cTransform);
};
