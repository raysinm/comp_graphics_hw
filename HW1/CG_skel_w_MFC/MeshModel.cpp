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

MeshModel::MeshModel(string fileName): _s_scales(1, 1, 1)
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

	vertex_positions = new vec3[3 * faces.size()]; /*BUG - fixed: each face is made of 3 vertecies.*/
	vertex_normals =   new vec3[3 * faces.size()];
	buffer2d =		   new vec2[3 * faces.size()]; //Worst case: each vertex is on a different pixel
	// iterate through all stored faces and create triangles
	int k=0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		FaceIdcs current = *it;
		for (int i = 0; i < 3; i++)
		{
			vertex_positions[k] = vertices[current.v[i] - 1];
			vertex_normals[k] = verticesNormals[current.vn[i] - 1];
			k++;
		}
	}

}

void MeshModel::draw(mat4& cTransform)
{
	//TODO: implement this function. i guess we should do all the transformation of "model-view" to update the m_outbuffer array...
	
	//---
	//--- Create transformation matrix
}

void MeshModel::scale(vec3& factors)
{
	auto scale_mat = Scale(factors.x, factors.y, factors.z);
	_model_transform = scale_mat * _model_transform;
}
void MeshModel::rotate(vec3& factors)
{
	auto scale_mat = Rotate(factors.x, factors.y, factors.z);
	_model_transform = scale_mat * _model_transform;
}
void MeshModel::translate(vec3& factors)
{
	auto scale_mat = Scale(factors.x, factors.y, factors.z);
	_model_transform = scale_mat * _model_transform;
}
void MeshModel::scaleInWorld(vec3& factors)
{
	auto scale_mat = Scale(factors.x, factors.y, factors.z);
	_model_transform = scale_mat * _model_transform;
}
void MeshModel::rotateInWorld(vec3& factors)
{
	auto scale_mat = Scale(factors.x, factors.y, factors.z);
	_model_transform = scale_mat * _model_transform;
}
void MeshModel::translateInWorld(vec3& factors)
{
	auto scale_mat = Scale(factors.x, factors.y, factors.z);
	_model_transform = scale_mat * _model_transform;
}

void transform(mat4& transform_matrix)
{
	_model_transform = transform_matrix * _model_transform;
}
