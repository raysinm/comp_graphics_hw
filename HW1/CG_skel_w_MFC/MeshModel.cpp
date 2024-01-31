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

vec2* MeshModel::Get2dBuffer()
{
	return buffer2d;
}

unsigned int MeshModel::Get2dBuffer_len()
{
	return num_vertices;
}

MeshModel::MeshModel(string fileName) : _scale(1,1,1,1), _scale_world(1,1,1,1)
{
	loadFile(fileName);
	// MAYBE; change _world_transform matrix:
	// TODO: Normalize all vertices
	
	// Default position is in 0,0,0
	// TODO: translate to user-defined position

	_world_transform = _model_transform = mat4();	// Initialize transform matrices

}

MeshModel::~MeshModel(void)
{
	if(vertex_positions)
		delete[] vertex_positions;
	if (t_vertex_positions)
		delete[] t_vertex_positions;
	if(vertex_normals)
		delete[] vertex_normals;

}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
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
			vertices.push_back(vec3fFromStream(issLine));
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

	//num_vertices = 3 * faces.size(); // comment for debug only
	num_vertices = 12;
	vertex_positions = new vec3[num_vertices]; /*BUG - fixed: each face is made of 3 vertecies.*/
	vertex_normals =   new vec3[num_vertices];
	buffer2d =		   new vec2[num_vertices]; //Worst case: each vertex is on a different pixel
	// iterate through all stored faces and create triangles
	//int k=0;
	//for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	//{
	//	FaceIdcs current = *it;
	//	for (int i = 0; i < 3; i++)
	//	{
	//		vertex_positions[k] = vertices[current.v[i] - 1];
	//		vertex_normals[k] = verticesNormals[current.vn[i] - 1];
	//		k++;
	//	}
	//}

#ifdef _DEBUG
	buffer2d[0] = vec2(0, 0);     
	buffer2d[1] = vec2(0.5, 0);   
	buffer2d[2] = vec2(0.5, 0.5); 

	buffer2d[3] = vec2(0, 0);	
	buffer2d[4] = vec2(0.5, 0);	
	buffer2d[5] = vec2(0.5, -0.5); 

	buffer2d[6] = vec2(0, 0);	
	buffer2d[7] = vec2(-0.5, 0);	
	buffer2d[8] = vec2(-0.5, -0.5); 

	buffer2d[9] = vec2(0, 0);	
	buffer2d[10] = vec2(0, 0.5); 
	buffer2d[11] = vec2(-0.5, 0);	
#endif
}

void MeshModel::draw(mat4& cTransform, mat4& projection)
{

	updateTransform();	//TODO maybe: Optimize to only update when changed;

	for (int i = 0; i < num_vertices; i++)
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
		mat3 cTransform_rot = TopLeft3(cTransform);
		mat3 cTransform_rot_inv = transpose(cTransform_rot);
		vec3 cTransfrom_trnsl = RightMostVec(cTransform);
		mat4 cTransform_inv(cTransform_rot_inv, -(cTransform_rot_inv * cTransfrom_trnsl));


		// Transform	
		v_i = projection * cTransform_inv * _world_transform * _model_transform * v_i;
		
		// Save result
		t_vertex_positions[i] = vec3(v_i.x, v_i.y, v_i.z);
		buffer2d[i] = vec2(v_i.x, v_i.y);	// Should be already normalized after projection

	}
}


void MeshModel::updateTransform()	// TODO: connect to rendering
{

	mat4 trnsl_m = Translate(_trnsl.x, _trnsl.y, _trnsl.z);
	mat4 rot_m_x = RotateX(_rot.x);
	mat4 rot_m_y = RotateY(_rot.y);
	mat4 rot_m_z = RotateZ(_rot.z);
	mat4 scale_m = Scale(_scale.x, _scale.y, _scale.z);

	_model_transform = scale_m * rot_m_z * rot_m_y * rot_m_x * trnsl_m;

}

