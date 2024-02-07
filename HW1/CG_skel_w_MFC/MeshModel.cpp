#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

struct FaceIdcs
{
	int v[4];   //vertex index
	int vn[4];  //vertex index from vertex normals array
	int vt[4];  //vertex texture i guess?...

	FaceIdcs()
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream & aStream)
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for(int i = 0; i < 3; i++)
		{
			aStream >> std::ws >> v[i] >> std::ws;
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> std::ws;
			if (aStream.peek() == '/')
			{
				aStream >> c >> std::ws >> vn[i];
				continue;
			}
			else
				aStream >> vt[i];
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vn[i];
		}
	}
};

vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

vec2* MeshModel::Get2dBuffer(MODEL_OBJECT obj)
{
	switch (obj)
	{
	case MODEL:
		return buffer2d;
	case BBOX:
		return buffer2d_bbox;
	case V_NORMAL:
		return buffer2d_v_normals;
	case F_NORMAL:
		return buffer2d_f_normals;
	}
}

unsigned int MeshModel::Get2dBuffer_len(MODEL_OBJECT obj)
{
	switch (obj)
	{
	case MODEL:
		return num_vertices;
	case BBOX:
		return num_bbox_vertices;
	case V_NORMAL:
		return num_vertices_unordered;
	case F_NORMAL:
		return num_faces;
	}
}

MeshModel::MeshModel()
{
	ResetAllUserTransforms();
}

MeshModel::MeshModel(string fileName) 
{
	loadFile(fileName);
	// Now we should have faces and their indices
	buffer2d = new vec2[num_vertices+num_bbox_vertices];
	for (int i = 0; i < num_vertices; i++)
	{
		vertex_positions[i] = vertex_positions_unordered[faces_v_indices[i]];
	}
	initBoundingBox();
	ResetAllUserTransforms();


}

MeshModel::~MeshModel(void)
{
	if(vertex_positions)
		delete[] vertex_positions;
	if (t_vertex_positions)
		delete[] t_vertex_positions;
	if(vertex_normals)
		delete[] vertex_normals;
	if (face_normals)
		delete[] face_normals;

	if (buffer2d)
		delete[] buffer2d;
	if (buffer2d_bbox)
		delete[] buffer2d_bbox;
	if (buffer2d_v_normals)
		delete[] buffer2d_v_normals;
	if (buffer2d_f_normals)
		delete[] buffer2d_f_normals;
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	//vector<vec3> vertices;
	vector<vec3> verticesNormals;

	while (!ifile.eof())
	{
		// get line
		string curLine;
		getline(ifile, curLine);

		// read type of the line
		istringstream issLine(curLine);
		string lineType;

		issLine >> std::ws >> lineType;

		// based on the type parse data
		if (lineType == "v") //Vertex  /*BUG - fixed*/
		{
			//vertices.push_back(vec3fFromStream(issLine));
			vertex_positions_unordered.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "f") //Face   /*BUG - fixed*/
		{
			faces.push_back(issLine);
		}
		else if (lineType == "vn") //Vector Normal
		{
			verticesNormals.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "#" || lineType == "")
		{
			// comment / empty line
		}
		else
		{
			cout << "Found unknown line Type \"" << lineType << "\"";
		}
	}
	
	//Vertex_positions is an array of vec3. Every three elements define a triangle in 3D.
	//If the face part of the obj is
	//f 1 2 3
	//f 1 3 4
	//Then vertex_positions should contain:
	//vertex_positions={v1,v2,v3,v1,v3,v4}
	num_faces = faces.size();
	num_vertices = 3 * num_faces;//Each face is made from 3 vertices
	//num_vertices_unordered = vertices.size();
	num_vertices_unordered = vertex_positions_unordered.size();

	vertex_positions = new vec3[num_vertices];
	vertex_normals	 = new vec3[num_vertices];
	t_vertex_positions = new vec3[num_vertices];
	//faces_indices = new vec3[num_faces];
	//vertex_positions_unordered = new vec3[num_vertices_unordered];

	bool v_normals_exist = verticesNormals.size() > 0;
	//iterate through all stored faces and create triangles

	
	for (auto face : faces)
	{
		for (int i = 0; i < 3; i++)
		{
			int v_index = face.v[i] - 1;
			faces_v_indices.push_back(v_index);

			if (v_normals_exist)
				vertex_normals[v_index] = verticesNormals[face.vn[i] - 1];
			
		}
	}

	//for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	//{
	//	FaceIdcs current_face = *it;
	//	for (int i = 0; i < 3; i++)
	//	{
	//		int v_index = current_face.v[i] - 1;
	//		faces_indices[k] = v_index;	// Saving vertex indices for later use
	//		vertex_faces_neighbors[v_index].push_back(k);	// Adding current face as neighbor to vertex (needed for normal calculation)
	//		// Now we want to keep for each vertex, all of its neighbors
	//									
	//		//vertex_positions[k] = vertices[current_face.v[i] - 1];	// 1-based indexing to 0-based indexing
	//		//vertex_positions[k] = vertices_positions_unordered[current_face.v[i] - 1];	// 1-based indexing to 0-based indexing

	//		if (v_normals_exist)
	//			vertex_normals[k]   = verticesNormals[current_face.vn[i] - 1];
	//		//k++;
	//	k++;	// new change
	//	}
	//}
	calculateFaceNormals();
	if (!v_normals_exist)
	{
		estimateVertexNormals();
	}


}

void MeshModel::initBoundingBox()
{
	// Assuming vertex_positions is initialized
	if (vertex_positions == nullptr) return;
	
	// Bounding box init
	b_box_vertices = new vec3[num_bbox_vertices];	// 12 triangular faces
	buffer2d_bbox = new vec2[num_bbox_vertices];
	// TODO: If changing to faces indices, use min(), max() in vector vertex_positions_unoredered maybe
		// Find min and max coordinates
	float min_x, min_y, min_z, max_x, max_y, max_z;
	min_x = max_x = vertex_positions[0].x;
	min_y = max_y = vertex_positions[0].y;
	min_z = max_z = vertex_positions[0].z;

	for (int i = 0; i < num_vertices; i++)
	{
		vec3 v = vertex_positions[i];
		if (v.x < min_x)	min_x = v.x;
		else if (v.x > max_x)	max_x = v.x;
		
		if (v.y < min_y)	min_y = v.y;
		else if (v.y > max_y)	max_y = v.y;
		
		if (v.z < min_z)	min_z = v.z;
		else if (v.z > max_z)	max_z = v.z;
	}
	std::vector<vec3> v = {
						   vec3(min_x, max_y, max_z),	//0
						   vec3(min_x, max_y, min_z),
						   vec3(max_x, max_y, min_z),
						   vec3(max_x, max_y, max_z),	
						   vec3(min_x, min_y, max_z),	
						   vec3(min_x, min_y, min_z),
						   vec3(max_x, min_y, min_z),
						   vec3(max_x, min_y, max_z)	//7
	};
	std::vector<GLint> indices = {
			0,1,2,	// Top
			0,2,3,

			4,5,6,	// Bottom
			4,6,7,

			0,1,5,	// Left
			0,4,5,

			2,3,7,	// Right
			2,6,7,

			0,4,7,	// Front
			0,3,7,

			1,2,6,	// Back
			1,5,6
	};

	for (int i = 0; i < num_bbox_vertices; i++)
	{
		b_box_vertices[i] = v[indices[i]];
	}


}

void MeshModel::calculateFaceNormals()
{
	int num_faces = num_vertices / 3;
	if (face_normals == nullptr)
	{
		face_normals = new vec3[num_faces];	// Should be int
		buffer2d_f_normals = new vec2[num_faces];
	}

	for (int i = 0; i < num_faces; i++)
	{
		//int k = i * 3;
		//vec3 v1 = vertex_positions[k + 1] - vertex_positions[k];
		//vec3 v2 = vertex_positions[k + 2] - vertex_positions[k];

		vec3 v_i = vertex_positions_unordered[faces_v_indices[i+0]];
		vec3 v_j = vertex_positions_unordered[faces_v_indices[i+1]];
		vec3 v_k = vertex_positions_unordered[faces_v_indices[i+2]];
		vec3 v_ji = v_j - v_i;
		vec3 v_ki = v_k - v_i;

		face_normals[i] = cross(v_ji, v_ki);
	}
}
void MeshModel::estimateVertexNormals()
{
	if (face_normals == nullptr)	return;
		//area : length(cross(v1,v2))/2;
	if (buffer2d_v_normals == nullptr)
		buffer2d_v_normals = new vec2[num_vertices_unordered];

	for (int i = 0; i < num_vertices_unordered; i++)
	{
		vector<int> neighbor_faces = vertex_faces_neighbors[i];
		vec3 v_normal(0);
		GLfloat area_tot = 0;
		for (auto neighbor_i : neighbor_faces)
		{
			vec3 f_normal = face_normals[neighbor_i];
			GLfloat area = length(f_normal);	// Should be cross product of face vectors TODO: CHECK
			v_normal += f_normal * area;
			area_tot += area;
		}
		// Maybe not neccesary:
		v_normal /= area_tot;
		vertex_normals[i] = v_normal;
	}
}

void MeshModel::draw(mat4& cTransform, mat4& projection)
{
	if (!userInitFinished) //Dont start to draw before user clicked 'OK' in the popup window...
		return;

	updateTransform();			//TODO Currently update only when changed
	updateTransformWorld();		//TODO Currently update only when changed

#ifdef _DEBUG
	//cout << "trnsl: " << _trnsl << endl;
	//cout << "trnsl w: " << _trnsl_w << endl;
	//cout << "rot: " << _rot << endl;
	//cout << "rot w: " << _rot_w << endl;
	//cout << "scale: " << _scale << endl;
	//cout << "scale w: " << _scale_w << endl;
#endif 


	mat3 cTransform_rot = TopLeft3(cTransform);
	mat3 cTransform_rot_inv = transpose(cTransform_rot);
	vec3 cTransfrom_trnsl = RightMostVec(cTransform);
	mat4 cTransform_inv(cTransform_rot_inv, -(cTransform_rot_inv * cTransfrom_trnsl));

	int i = 0;
	for (; i < num_vertices; i++)
	{

		// Homogenous vector for calculations:
		vec4 v_i(vertex_positions[i]);

		// Camera: ASSUMPTION: No scaling on camera
		//	cTransform =	 [ R     T ]
		//					 [ 0     1 ]
		// 
		// cTransform_inv =	 [ R^t  -R^t*T]
		//					 [ 0	 1	  ]
		//

		// Transform	
		v_i = projection * (cTransform_inv * (_world_transform * (_model_transform * v_i))); //This way we always multiply Matrix x Vector (O(N^2) per multiplication)
		
		// Save result
		t_vertex_positions[i] = vec3(v_i.x, v_i.y, v_i.z); //We dont really use this... 


		buffer2d[i] = vec2(v_i.x, v_i.y);	// Should be already normalized after projection

	}

	// Bounding box
	if (showBoundingBox)
	{
		for (int j = 0; j < num_bbox_vertices; j++)
		{
			vec4 v_j(b_box_vertices[j]);
			v_j = projection * (cTransform_inv * (_world_transform * (_model_transform * v_j))); //This way we always multiply Matrix x Vector (O(N^2) per multiplication)

			buffer2d_bbox[j] = vec2(v_j.x, v_j.y);
		}
	}
}

void MeshModel::updateTransform()
{
	mat4 trnsl_m = Translate(_trnsl.x, _trnsl.y, _trnsl.z);
	mat4 rot_m_x = RotateX(_rot.x);
	mat4 rot_m_y = RotateY(_rot.y);
	mat4 rot_m_z = RotateZ(_rot.z);
	mat4 scale_m = Scale(_scale.x, _scale.y, _scale.z);	


	_model_transform = scale_m * rot_m_z * rot_m_y * rot_m_x * trnsl_m; // TEST IF ORDER MATTERS

}

void MeshModel::updateTransformWorld()
{
	mat4 trnsl_m = Translate(_trnsl_w.x, _trnsl_w.y, _trnsl_w.z);
	mat4 rot_m_x = RotateX(_rot_w.x);
	mat4 rot_m_y = RotateY(_rot_w.y);
	mat4 rot_m_z = RotateZ(_rot_w.z);
	mat4 scale_m = Scale(_scale_w.x, _scale_w.y, _scale_w.z);

	_world_transform = scale_m * rot_m_z * rot_m_y * rot_m_x * trnsl_m; // TEST IF ORDER MATTERS
}

void MeshModel::setTranslation(vec3& trnsl)
{
	_trnsl = trnsl;
}

void MeshModel::setRotation(GLfloat rot, char axis)
{
	switch (axis)
	{
	case 'x':
		_rot.x = rot;
		break;
	case 'y':
		_rot.y = rot;
		break;
	case 'z':
		_rot.z = rot;
		break;
	}
}

void MeshModel::setScale(vec3& scale)
{
	_scale = scale;
}

void MeshModel::setTranslationWorld(vec3& trnsl)
{
	_trnsl_w = trnsl;
}

void MeshModel::setRotationWorld(GLfloat rot, char axis)
{
	switch (axis)
	{
	case 'x':
		_rot_w.x = rot;
		break;
	case 'y':
		_rot_w.y = rot;
		break;
	case 'z':
		_rot_w.z = rot;
		break;
	}
}

void MeshModel::setScaleWorld(vec3& scale)
{
	_scale_w = scale;
}

void MeshModel::ResetAllUserTransforms()
{
	_trnsl = vec4(0);
	_rot = vec4(0);
	_scale = vec4(1);

	_trnsl_w = vec4(0);
	_rot_w = vec4(0);
	_scale_w = vec4(1);
}

void MeshModel::ResetUserTransform_translate_model()
{
	_trnsl = vec4(0);
}

void MeshModel::ResetUserTransform_rotate_model()
{
	_rot = vec4(0);
}

void MeshModel::ResetUserTransform_scale_model()
{
	_scale = vec4(1);
}

void MeshModel::ResetUserTransform_translate_world()
{
	_trnsl_w = vec4(0);
}

void MeshModel::ResetUserTransform_rotate_world()
{
	_rot_w = vec4(0);
}

void MeshModel::ResetUserTransform_scale_world()
{
	_scale_w = vec4(1);
}