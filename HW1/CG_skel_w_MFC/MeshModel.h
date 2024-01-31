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
<<<<<<< HEAD
	vec3* vertex_positions;	// In model space- model transformations are applied immidiatly
	int num_vertices;	//TODO
	vec3* t_vertex_positions;	// Transformed	- Used for pipeline	- No z axis
=======
	vec3* vertex_positions;		// In model space 
	vec3* t_vertex_positions;	// Transformed	- Used for pipeline
>>>>>>> 311729995b7bda6bfd50df9a3921a1ad885bd5cd
	vec3* vertex_normals;
	vec2* buffer2d;				//Use this buffer to send the renederer for Rasterazation process.   Initiate once, update each frame.

	mat4 _world_transform;	
	mat4 _model_transform;	
	vec4 _trnsl, _rot, _scale, _trnsl_world, _rot_world, _scale_world;
	
	mat3 _normal_transform;

public:
	vec2* Get2dBuffer();
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void MeshModel::draw(mat4& cTransform)
	{


	}
	
	void updateTransform()
	{
		mat4 trnsl_m = Translate(_trnsl.x, _trnsl.y, _trnsl.z);
		mat4 rot_m_x = RotateX(_rot.x);
		mat4 rot_m_y = RotateY(_rot.y);
		mat4 rot_m_z = RotateZ(_rot.z);
		mat4 scale_m = Scale(_scale.x, _scale.y, _scale.z);

		

	}
	//void setTranslation(vec3& )

	//void scale(vec3& factors);
	//void rotate(vec3& factors);
	//void translate(vec3& factors);
	//void scaleInWorld(vec3& factors);
	//void rotateInWorld(vec3& factors);
	//void translateInWorld(vec3& factors);



};
