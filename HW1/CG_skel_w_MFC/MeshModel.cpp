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
		return num_vertices_to_draw;
	case BBOX:
		return num_bbox_vertices;
	case V_NORMAL:
		return num_vertices_raw*2;
	case F_NORMAL:
		return num_faces*2;

	default:
		return -1;
	}
}

MeshModel::MeshModel()
{
	ResetAllUserTransforms();
}

MeshModel::MeshModel(string fileName) 
{
	ResetAllUserTransforms();
	
	loadFile(fileName);
	
	initBoundingBox();

	buffer2d = new vec2[num_vertices];

}

MeshModel::~MeshModel(void)
{
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
		if (lineType == "v") //Vertex
		{
			vertex_positions_raw.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "f") //Face
		{
			faces.push_back(issLine);
		}
		else if (lineType == "vn") //Vertex Normal
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
	
	num_vertices_raw = vertex_positions_raw.size();
	num_faces = faces.size();
	num_vertices = 3 * num_faces;			//Each face is made from 3 vertices

	t_vertex_positions_normalized	= vector<vec3> (num_vertices_raw);
	vertex_normals					= vector<vec3> (num_vertices_raw);
	vertex_faces_neighbors			= vector<vector<int>> (num_vertices_raw);
	buffer2d_v_normals				= new vec2[num_vertices_raw * 2];
	buffer2d_f_normals				= new vec2[num_faces * 2];


	bool v_normals_exist = verticesNormals.size() > 0;

	//iterate through all stored faces and create triangles
	unsigned int face_id = 0;
	for (auto face : faces)
	{
		for (int i = 0; i < 3; i++)
		{
			faces_v_indices.push_back(face.v[i] - 1);
			vertex_faces_neighbors[face.v[i] - 1].push_back(face_id);
		}
		face_id++;
	}

		
	calculateFaceNormals();

	if (!v_normals_exist)
	{
		estimateVertexNormals();
	}
	else
	{
		for (auto face : faces)
			for (int i = 0; i < 3; i++)
				vertex_normals[face.v[i] - 1] = verticesNormals[face.vn[i] - 1];
	}



}

void MeshModel::initBoundingBox()
{
	if (vertex_positions_raw.empty()) return;
	
	// Bounding box init
	b_box_vertices = vector<vec3> (num_bbox_vertices);
	buffer2d_bbox = new vec2      [num_bbox_vertices];

	float min_x, min_y, min_z, max_x, max_y, max_z;
	min_x = max_x = vertex_positions_raw[0].x;
	min_y = max_y = vertex_positions_raw[0].y;
	min_z = max_z = vertex_positions_raw[0].z;

	for (int i = 0; i < num_vertices_raw; i++)
	{
		vec3 v = vertex_positions_raw[i];
		if (v.x < min_x)		min_x = v.x;
		else if (v.x > max_x)	max_x = v.x;
		
		if (v.y < min_y)		min_y = v.y;
		else if (v.y > max_y)	max_y = v.y;
		
		if (v.z < min_z)		min_z = v.z;
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

	for (unsigned int i = 0; i < num_bbox_vertices; i++)
		b_box_vertices[i] = v[indices[i]];
}

void MeshModel::calculateFaceNormals()
{	
	for (int i = 0; i < num_faces; i++)
	{
		vec3 v_i = vertex_positions_raw[faces_v_indices[(i * 3) + 0]];
		vec3 v_j = vertex_positions_raw[faces_v_indices[(i * 3) + 1]];
		vec3 v_k = vertex_positions_raw[faces_v_indices[(i * 3) + 2]];

		vec3 v_ji = v_j - v_i;
		vec3 v_ki = v_k - v_i;

		face_normals.push_back( normalize(cross(v_ji, v_ki)) );
	}
}

void MeshModel::estimateVertexNormals()
{
	for (unsigned int i = 0; i < num_vertices_raw; i++)
	{
		vector<int> neighbor_faces = vertex_faces_neighbors[i];

		vec3 v_normal(0);

		for (auto neighbor_i : neighbor_faces)
			v_normal += face_normals[neighbor_i];
		
		vertex_normals[i] = normalize(v_normal);
	}
}

void MeshModel::draw(mat4& cTransform, mat4& projection, bool allowClipping, mat4& cameraRot)
{
	if (!userInitFinished) //Dont start to draw before user clicked 'OK' in the popup window...
		return;

	updateTransform();	
	updateTransformWorld();
	updateTransformCombinedInv();

	//Apply all transformations and save in t_vertex_positions_normalized array
	for (unsigned int i = 0; i < vertex_positions_raw.size(); i++)
	{
		vec4 v_i(vertex_positions_raw[i]);

		//Apply model-view transformation
		v_i = cTransform * (_world_transform * (_model_transform * v_i));

		//Apply projection:
		v_i = projection * v_i;
		
		// Save result
		t_vertex_positions_normalized[i] = vec3(v_i.x, v_i.y, v_i.z) / v_i.w;
	}

	// Model buffer - Add to 2d-buffer all vertecies
	// Clipping
	num_vertices_to_draw = 0;
	int buffer_i = 0;
	vector<int> faces_to_draw;
	for (unsigned int face_indx = 0; face_indx < num_faces; face_indx++)
	{
		bool atleast_one_vertex_in_bound = false;
		if (allowClipping)
		{
			/*	Check if ATLEAST 1 vertex is in-bound. foreach dimension: -1<x<1
				If yes: Add the face to buffer2d
				else: Don't add the face*/
			for (unsigned int v = 0; v < 3; v++)
			{
				vec3 point = t_vertex_positions_normalized[faces_v_indices[(face_indx * 3) + v]];

				if (point.x >= -1 && point.x <= 1 &&
					point.y >= -1 && point.y <= 1 &&
					point.z >= -1 && point.z <= 1)
				{
					atleast_one_vertex_in_bound = true;
					faces_to_draw.push_back(face_indx);
					break;
				}
			}
		}
		else
		{
			faces_to_draw.push_back(face_indx);
		}

		/* add the 3 points of the current face: */
		if (atleast_one_vertex_in_bound || !allowClipping)
		{
			for (unsigned int v = 0; v < 3; v++)
			{
				vec3 point = t_vertex_positions_normalized[faces_v_indices[(face_indx * 3) + v]];
				buffer2d[(buffer_i * 3) + v] = vec2(point.x, point.y);
				num_vertices_to_draw++;
			}
			buffer_i++;
		}

	}
	num_faces_to_draw = faces_to_draw.size();

	// Bounding box buffer
	if (showBoundingBox)
	{
		for (unsigned int j = 0; j < num_bbox_vertices; j++)
		{
			vec4 v_j(b_box_vertices[j]);

			//Apply model-view transformations:
			v_j = cTransform * (_world_transform * (_model_transform * v_j));

			//Project:
			v_j = projection * v_j;

			//Add to 2d buffer: 
			buffer2d_bbox[j] = vec2(v_j.x, v_j.y) / v_j.w;
		}
	}

	// Vertex normals buffer
	if (showVertexNormals)
	{
		for (unsigned int j = 0; j < num_vertices_raw; j++)
		{
			vec4 v_j (vertex_normals[j]);

			//Transform the normal vector:
			v_j = cameraRot * (_world_transform_for_normals * (_model_transform_for_normals * v_j));

			//Project the vector:
			v_j = projection * v_j;

			//Make sure it is still normalized:
			v_j = normalize(v_j);

			vec4 start_point = t_vertex_positions_normalized[j];
			vec4 end_point	 = start_point + (v_j * length_vertex_normals);

			buffer2d_v_normals[j * 2 + 0] = vec2(start_point.x, start_point.y);
			buffer2d_v_normals[j * 2 + 1] = vec2(end_point.x, end_point.y);
		}
	}
	
	// Face normals buffer
	if (showFaceNormals)
	{
		unsigned int buffer_i = 0;
		//for (int face_indx : faces_to_draw)
		for (unsigned int face_indx = 0; face_indx < num_faces; face_indx++)
		{
			vec4 v_n(face_normals[face_indx]);

			//Transform the normal vector:
			v_n = cameraRot * (_world_transform_for_normals * (_model_transform_for_normals * v_n));
			
			//Project the vector:
			v_n = projection * v_n;

			//Make sure it is still normalized:
			v_n = normalize(v_n);

			// Cacluate center of face as the start point:
			vec3 v0 = t_vertex_positions_normalized[faces_v_indices[(face_indx * 3) + 0]];
			vec3 v1 = t_vertex_positions_normalized[faces_v_indices[(face_indx * 3) + 1]];
			vec3 v2 = t_vertex_positions_normalized[faces_v_indices[(face_indx * 3) + 2]];

			vec3 start_point = vec3(v0 + v1 + v2) / 3;
			vec3 end_point = start_point + (vec3(v_n.x, v_n.y, v_n.z) * length_face_normals);


			buffer2d_f_normals[ (buffer_i * 2) + 0] = vec2(start_point.x, start_point.y);
			buffer2d_f_normals[ (buffer_i * 2) + 1] = vec2(end_point.x, end_point.y);
			++buffer_i;
		}
	}
}

void MeshModel::updateTransformCombinedInv()
{
	mat4 rot_m_x = transpose(RotateX(_rot_w.x + _rot.x));
	mat4 rot_m_y = transpose(RotateY(_rot_w.y + _rot.y));
	mat4 rot_m_z = transpose(RotateZ(_rot_w.z + _rot.z));
	mat4 trnsl_m = Translate(-_trnsl - _trnsl_w);
	mat4 scale_m = Scale(1/(_scale_w.x+_scale.x), 1 / (_scale_w.y + _scale.y), 1 / (_scale_w.z + _scale.z));

	combined_inv = scale_m * (trnsl_m * (rot_m_z * (rot_m_y * rot_m_x)));
}


void MeshModel::updateTransform()
{
	mat4 rot_m_x = RotateX(_rot.x);
	mat4 rot_m_y = RotateY(_rot.y);
	mat4 rot_m_z = RotateZ(_rot.z);
	mat4 trnsl_m = Translate(_trnsl);
	mat4 scale_m = Scale(_scale.x, _scale.y, _scale.z);	
	mat4 scale_inverse_m = Scale(1/_scale.x, 1/_scale.y, 1/_scale.z);	

	_model_transform = scale_m * (trnsl_m * (rot_m_z * (rot_m_y * rot_m_x)));
	_model_transform_for_normals = rot_m_z * (rot_m_y * rot_m_x);

	//// Inverse
	//mat4 rot_m_x_inv = transpose(rot_m_x);
	//mat4 rot_m_y_inv = transpose(rot_m_y);
	//mat4 rot_m_z_inv = transpose(rot_m_z);
	//mat4 trnsl_m_inv = Translate(- _trnsl);	
	//mat4 scale_m_inv = Scale(1/_scale.x, 1/_scale.y, 1/_scale.z);
	//_model_transform_inv = scale_m_inv * (trnsl_m_inv *(rot_m_z_inv * (rot_m_y_inv * rot_m_x_inv)));
	//updateTransformCombinedInv();

}

void MeshModel::updateTransformWorld()
{
	mat4 rot_m_x = RotateX(_rot_w.x);
	mat4 rot_m_y = RotateY(_rot_w.y);
	mat4 rot_m_z = RotateZ(_rot_w.z);
	mat4 trnsl_m = Translate(_trnsl_w.x, _trnsl_w.y, _trnsl_w.z);
	mat4 scale_m = Scale(_scale_w.x, _scale_w.y, _scale_w.z);
	mat4 scale_inverse_m = Scale(1/_scale_w.x, 1/_scale_w.y, 1/_scale_w.z);
	
	_world_transform = scale_m * (trnsl_m * (rot_m_z * (rot_m_y * rot_m_x)));
	_world_transform_for_normals = rot_m_z * (rot_m_y * rot_m_x);
	


	//// Inverse
	//mat4 rot_m_x_inv = transpose(rot_m_x);
	//mat4 rot_m_y_inv = transpose(rot_m_y);
	//mat4 rot_m_z_inv = transpose(rot_m_z);
	//mat4 trnsl_m_inv = Translate(- _trnsl_w);
	//mat4 scale_m_inv = Scale(1 / _scale_w.x, 1 / _scale_w.y, 1 / _scale_w.z);
	//_world_transform_inv = scale_m_inv * (trnsl_m_inv *(rot_m_z_inv * (rot_m_y_inv * rot_m_x_inv)));
	//updateTransformCombinedInv();

}


vec4 MeshModel::getCenterOffMass()
{
/* In world space */

	//updateTransform();
	//updateTransformWorld();

	//vec4 c(0);
	//for (auto p : vertex_positions_raw)
	//	c += _world_transform * (_model_transform * p);

	//c /= vertex_positions_raw.size();
	//c.w = 1;


	//return vec4(c);
	vec4 res = _trnsl_w + _trnsl;
	res.w = 1;
	return res;
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
	_trnsl = vec4(0, 0, 0);
	_rot = vec4(0, 0, 0);
	_scale = vec4(1);

	_trnsl_w = vec4(0, 0, 0);
	_rot_w = vec4(0, 0, 0);
	_scale_w = vec4(1);
}

void MeshModel::ResetUserTransform_translate_model()
{
	_trnsl = vec4(0,0,0);
}

void MeshModel::ResetUserTransform_rotate_model()
{
	_rot = vec4(0, 0, 0);
}

void MeshModel::ResetUserTransform_scale_model()
{
	_scale = vec4(1);
}

void MeshModel::ResetUserTransform_translate_world()
{
	_trnsl_w = vec4(0, 0, 0);
}

void MeshModel::ResetUserTransform_rotate_world()
{
	_rot_w = vec4(0, 0, 0);
}

void MeshModel::ResetUserTransform_scale_world()
{
	_scale_w = vec4(1);
}