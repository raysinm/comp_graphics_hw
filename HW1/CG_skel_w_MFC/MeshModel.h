#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

using namespace std;

class MeshModel : public Model
{
protected :
	MeshModel() {}
	vec3* vertex_positions;
	vec3* vertex_normals;

	
	mat4 _world_transform;
	mat3 _normal_transform;

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw();
	
};
