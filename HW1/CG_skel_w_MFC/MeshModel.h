#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

enum MODEL_OBJECT{
	MODEL,
	BBOX,
	V_NORMAL,
	F_NORMAL
};
using namespace std;

class MeshModel : public Model
{
protected:
	friend class Scene;

	MeshModel();
	
	vector<vec3> vertex_positions_raw;				//Raw data from .obj file.
	vector<vec3> t_vertex_positions_normalized;		//In Camera space, normalized to [-1, 1]

	vector<int> faces_v_indices;					//Each 3 indices makes a face. (triangle)
	vector<vector<int>> vertex_faces_neighbors;		//Used for calculating the vertex normals.

	vector<vec3> vertex_normals;					// size: Num of vertices ("raw")
	vector<vec3> face_normals;						// size: Num of faces
	vector<vec3> b_box_vertices;


	unsigned int num_vertices;
	unsigned int num_vertices_raw;
	unsigned int num_faces;
	unsigned int num_faces_to_draw;
	unsigned int num_vertices_to_draw;
	const unsigned int num_bbox_vertices = 36;
	float length_face_normals   = 1.0f;
	float length_vertex_normals = 1.0f;
	
	vec2* buffer2d = nullptr;					//Use this buffer to send the renederer for Rasterazation process.   Initiate once, update each frame.
	vec2* buffer2d_bbox = nullptr;				//Use this buffer to send the renederer for Rasterazation process.   Initiate once, update each frame.
	vec2* buffer2d_v_normals = nullptr;			//Use this buffer to send the renederer for Rasterazation process.   Initiate once, update each frame.
	vec2* buffer2d_f_normals = nullptr;			//Use this buffer to send the renederer for Rasterazation process.   Initiate once, update each frame.

	mat4 _world_transform;
	mat4 _model_transform;
	mat4 combined_inv;
	mat4 _world_transform_for_normals;
	mat4 _model_transform_for_normals;		

	mat3 _normal_transform;		// FOR NORMALS!!! G = (M^(-1))^T


	void initBoundingBox();
	void calculateFaceNormals();
	void estimateVertexNormals();

public:
	vec4 _trnsl, _rot, _scale;			// Model space
	vec4 _trnsl_w, _rot_w, _scale_w;	// World space

	bool showVertexNormals	= false;
	bool showFaceNormals	= false;
	bool showBoundingBox    = false;

	MeshModel(string fileName);
	~MeshModel(void);

	vec2* Get2dBuffer(MODEL_OBJECT obj);
	unsigned int Get2dBuffer_len(MODEL_OBJECT obj);

	void loadFile(string fileName);
	void draw(mat4& cTransform, mat4& projection, bool allowClipping, mat4& cameraRot);

	void updateTransform();
	void updateTransformWorld();
	void updateTransformCombinedInv();

	vec4 getCenterOffMass();

	float* getLengthFaceNormal()   { return &length_face_normals; }
	float* getLengthVertexNormal() { return &length_vertex_normals; }

	//Model space:
	void setTranslation(vec3& trnsl);
	void setRotation(GLfloat rot, char axis);
	void setScale(vec3& scale);

	//World space:
	void setTranslationWorld(vec3& trnsl);
	void setRotationWorld(GLfloat rot, char axis);
	void setScaleWorld(vec3& scale);

	//Gui helper functions:
	void ResetAllUserTransforms();
	void ResetUserTransform_translate_model();
	void ResetUserTransform_rotate_model();
	void ResetUserTransform_scale_model();
	void ResetUserTransform_translate_world();
	void ResetUserTransform_rotate_world();
	void ResetUserTransform_scale_world();

};
