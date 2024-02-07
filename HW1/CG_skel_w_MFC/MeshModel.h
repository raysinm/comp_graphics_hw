#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

#define FINDEX(face) face*3

using namespace std;

class MeshModel : public Model
{
protected:
	MeshModel();
	vec3* vertex_positions = nullptr;	// In model space- model transformations are applied immidiatly
	//vec3* vertex_positions_unordered = nullptr;
	vector<vec3> vertex_positions_unordered;
	vector<int> faces_v_indices;
	vector<vector<int>> vertex_faces_neighbors;

	vec3* b_box_vertices = nullptr;
	unsigned int num_vertices;
	unsigned int num_vertices_unordered;
	unsigned int num_faces;
	unsigned int num_bbox_vertices = 36;
	
	vec3* t_vertex_positions = nullptr;	// Transformed	- Used for pipeline	- No z axis
	vec3* vertex_normals = nullptr;
	vec3* face_normals = nullptr;

	vec2* buffer2d = nullptr;			//Use this buffer to send the renederer for Rasterazation process.   Initiate once, update each frame.

	mat4 _world_transform;
	mat4 _model_transform;

	mat3 _normal_transform;	// ??

	friend class Scene;

	void initBoundingBox();
	void calculateFaceNormals();
	void estimateVertexNormals();

public:
	vec4 _trnsl, _rot, _scale;			// Model space
	vec4 _trnsl_w, _rot_w, _scale_w;	// World space
	bool showVertexNormals	= false;
	bool showFaceNormals	= false;
	bool showBoundingBox = false;

	vec2* Get2dBuffer();
	unsigned int Get2dBuffer_len();
	MeshModel(string fileName);	// Add option to give initial world position 
	~MeshModel(void);
	void loadFile(string fileName);
	void MeshModel::draw(mat4& cTransform, mat4& projection);

	void MeshModel::updateTransform();
	void MeshModel::updateTransformWorld();

	//Model space:
	void setTranslation(vec3& trnsl);
	void setRotation(GLfloat rot, char axis);
	void setScale(vec3& scale);

	//World space:
	void setTranslationWorld(vec3& trnsl);
	void setRotationWorld(GLfloat rot, char axis);
	void setScaleWorld(vec3& scale);

	void ResetAllUserTransforms();
	void ResetUserTransform_translate_model();
	void ResetUserTransform_rotate_model();
	void ResetUserTransform_scale_model();

	void ResetUserTransform_translate_world();
	void ResetUserTransform_rotate_world();
	void ResetUserTransform_scale_world();

};
