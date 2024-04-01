#pragma once
#include "CG_skel_w_glfw.h"
#include "Scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include <chrono>
#include "Material.h"

using namespace std;



class MeshModel : public Model
{
protected:
	friend class Scene;

	MeshModel(Renderer* rend = nullptr);
	void GenerateVBO_WireFrame();
	void GenerateVBO_Triangles();
	void GenerateVBO_BBox();
	void GenerateVBO_vNormals();
	void GenerateVBO_fNormals();
	void GenerateAllGPU_Stuff();
	void GenerateTextures();
	vector<vec3> duplicateEachElement(const vector<vec3>& v, const int duplicateNumber = 2);

	vector<vec3> vertex_positions_raw;					 // Raw data from .obj file.
	vector<vec3> vertex_positions_triangle_gpu;			 // Vertex positions data in triangle form
	vector<vec3> vertex_face_positions_triangle_gpu;	 // Face positions data in triangle form
	vector<vec3> vertex_vn_triangle_gpu;				 // Vertex Normal direction in triangle form
	vector<vec3> vertex_fn_triangle_gpu;				 // Face Normal direction in triangle form
	vector<vec3> vertex_diffuse_color_flat_triangle_gpu; // Non-Uniform color in triangle form
	vector<vec3> vertex_diffuse_color_triangle_gpu;		 // Non-Uniform color in triangle form
	vector<vec3> vertex_positions_wireframe_gpu;		 // Vertex positions data in wireframe form
	vector<vec3> vertex_positions_bbox_gpu;
	vector<vec3> vertex_positions_Vnormals_gpu;
	vector<vec3> vertex_directions_Vnormals_gpu;
	vector<vec3> vertex_positions_Fnormals_gpu;
	vector<vec3> vertex_directions_Fnormals_gpu;
	vector<vec2> verticesTextures_gpu;

	vector<int> faces_v_indices;					//Each 3 indices makes a face. (triangle)
	vector<vector<int>> vertex_faces_neighbors;		//Used for calculating the vertex normals.
	vector<vec3> vertex_normals;					// size: Num of vertices ("raw")
	vector<vec3> vertex_normals_viewspace;			// size: Num of faces
	vector<vec3> face_normals;						// size: Num of faces
	vector<vec3> face_normals_viewspace;			// size: Num of faces


	chrono::high_resolution_clock::time_point start_time;

	vector<Material> materials;
	Material userDefinedMaterial;

	unsigned int num_vertices_raw;
	unsigned int num_faces;
	unsigned int num_vertices_to_draw;
	const unsigned int num_bbox_vertices = 24;
	float length_face_normals   = 1.0f;
	float length_vertex_normals = 1.0f;
	float min_x, min_y, min_z, max_x, max_y, max_z;
	

	mat4 _world_transform;
	mat4 _model_transform;
	mat4 _world_transform_for_normals;
	mat4 _model_transform_for_normals;

	mat4 model_view_mat;
	mat4 model_view_mat_for_normals;

	void initBoundingBox();
	void calculateFaceNormals();
	void estimateVertexNormals();
	void CreateVertexVectorForGPU();

	Renderer* renderer;

public:
	vec4 _trnsl, _rot, _scale;			// Model space
	vec4 _trnsl_w, _rot_w, _scale_w;	// World space
	GLuint tex = 0;
	STB_Image textureMap = { 0 };
	ColorAnimationType colorAnimationType = COLOR_ANIMATION_STATIC;

	bool showVertexNormals		= false;
	bool showFaceNormals		= false;
	bool showBoundingBox		= false;
	bool nonUniformDataUpdated	= false;
	bool isUniformMaterial		= true;
	bool vertexAnimationEnable  = false;


	MeshModel(string fileName, Renderer* rend = nullptr);
	~MeshModel(void);

	vector<vec3>* getVertexNormals() { return &vertex_normals; }
	vector<vec3>* getVertexNormalsViewSpace() { return &vertex_normals_viewspace; }
	vector<vec3>* getFaceNormals() { return   &face_normals; }
	vector<vec3>* getFaceNormalsViewSpace() { return   &face_normals_viewspace; }
	Vertex* GetBuffer();
	vec2* GetBuffer(MODEL_OBJECT obj);
	unsigned int GetBuffer_len(MODEL_OBJECT obj);
	void PopulateNonUniformColorVectorForGPU();
	void loadFile(string fileName);
	void draw(mat4& cTransform, mat4& projection, bool allowClipping, mat4& cameraRot);

	void updateTransform();
	void updateTransformWorld();
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
	
	void GenerateMaterials();
	vector<Material>& getMaterials() { return materials; }
	Material& getUserDefinedMaterial() { return userDefinedMaterial; }

	void UpdateModelViewInGPU(mat4& Tc, mat4& Tc_for_normals);
	void UpdateMaterialinGPU();
	void UpdateTextureInGPU();
	void UpdateAnimationInGPU();


};
