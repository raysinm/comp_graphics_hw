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
	MeshModel();
	vec3* vertex_positions = nullptr;	// In model space- model transformations are applied immidiatly
	unsigned int num_vertices;
	vec3* t_vertex_positions = nullptr;	// Transformed	- Used for pipeline	- No z axis
	vec3* vertex_normals = nullptr;
	vec2* buffer2d = nullptr;			//Use this buffer to send the renederer for Rasterazation process.   Initiate once, update each frame.

	mat4 _world_transform;	
	mat4 _model_transform;	
	vec4 _trnsl, _rot, _scale;
	vec4 _trnsl_w, _rot_w, _scale_w;	// Model transform
	//vec4 _trnsl_w, _rot_w, _scale_w, _trnsl_world_w, _rot_world_w, _scale_world_w;	// World transform

	mat3 _normal_transform;

	friend class Scene;

public:
	vec2* Get2dBuffer();
	unsigned int Get2dBuffer_len();
	MeshModel(string fileName);	// Add option to give initial world position 
	~MeshModel(void);
	void loadFile(string fileName);
	void MeshModel::draw(mat4& cTransform, mat4& projection);
	
	void MeshModel::updateTransform();
	void MeshModel::updateTransformWorld();

	//--- Interface for triggering transformation on a model
	void setTranslation(MeshModel* model, vec3& trnsl)
	{
		model->_trnsl = trnsl;
		updateTransform();
	}
	void setRotation(MeshModel* model, GLfloat rot, char axis)
	{
		switch (axis)
		{
		case 'x':
			model->_rot.x = rot;
			break;
		case 'y':
			model->_rot.y = rot;
			break;
		case 'z':
			model->_rot.z = rot;
			break;
		}
		updateTransform();
	}
	void setScale(MeshModel* model, vec3& scale)
	{
		model->_scale = scale;
		updateTransform();
	}
	void setTranslationWorld(MeshModel* model, vec3& trnsl)
	{
		model->_trnsl_w = trnsl;
		updateTransformWorld();
	}
	void setRotationWorld(MeshModel* model, GLfloat rot, char axis)
	{
		switch (axis)
		{
		case 'x':
			model->_rot_w.x = rot;
			break;
		case 'y':
			model->_rot_w.y = rot;
			break;
		case 'z':
			model->_rot_w.z = rot;
			break;
		}
		updateTransformWorld();
	}
	void setScaleWorld(MeshModel* model, vec3& scale)
	{
		model->_scale_w = scale;
		updateTransformWorld();
	}
	//void scale(vec3& factors);
	//void rotate(vec3& factors);
	//void translate(vec3& factors);
	//void scaleInWorld(vec3& factors);
	//void rotateInWorld(vec3& factors);
	//void translateInWorld(vec3& factors);



};
