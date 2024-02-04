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

MeshModel::MeshModel()
{
	ResetAllUserTransforms();
}

MeshModel::MeshModel(string fileName) 
{
	loadFile(fileName);
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

	num_vertices = 3 * faces.size(); //Each face is made from 3 vertices
	vertex_positions = new vec3[num_vertices];
	vertex_normals	 = new vec3[num_vertices];
	buffer2d		 = new vec2[num_vertices];
	t_vertex_positions = new vec3[num_vertices];
	
	//iterate through all stored faces and create triangles
	int k=0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		FaceIdcs current = *it;
		for (int i = 0; i < 3; i++)
		{
			vertex_positions[k] = vertices[current.v[i] - 1];
			vertex_normals[k]   = verticesNormals[current.vn[i] - 1];
			k++;
		}
	}

#ifdef _DEBUG
/*	buffer2d[0] = vec2(0, 0);     
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
	buffer2d[11] = vec2(-0.5, 0);*/	
#endif
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

#ifdef _DEBUG
	// Test new mat funcs
		//mat4 mat = mat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
		//cout << "mat" << mat << endl;
		//mat3 topleft3 = TopLeft3(mat);
		//cout << "topleft3" << topleft3 << endl;
		//vec3 r_vec = RightMostVec(mat);
		//cout << "r_vec" << r_vec << endl;
		//mat3 trans_m = transpose(topleft3);
		//cout << "trans_m" << trans_m << endl;
		//mat4 res(trans_m, -(trans_m * r_vec));
		//cout << "res" << res << endl;


#endif
		mat3 cTransform_rot = TopLeft3(cTransform);
		mat3 cTransform_rot_inv = transpose(cTransform_rot);
		vec3 cTransfrom_trnsl = RightMostVec(cTransform);
		mat4 cTransform_inv(cTransform_rot_inv, -(cTransform_rot_inv * cTransfrom_trnsl));


		// Transform	
		v_i = projection * (cTransform_inv * (_world_transform * (_model_transform * v_i))); //This way we always multiply Matrix x Vector (O(N^2) per multiplication)
		
		// Save result
		t_vertex_positions[i] = vec3(v_i.x, v_i.y, v_i.z); //We dont really use this... 

		buffer2d[i] = vec2(v_i.x, v_i.y);	// Should be already normalized after projection

		//cout << "vertex_positions: ";
		//for (int j = 0; j <= i; j++)
		//{
		//	cout << vertex_positions[j];
		//}
		//cout << endl << endl;
		//cout << "t_vertex_positions: ";
		//for (int j = 0; j <= i; j++)
		//{
		//	cout << t_vertex_positions[j];
		//}
		//cout << endl << endl;
		//cout << "buffer2d: ";
		//for (int j = 0; j<=i; j++)
		//{
		//	cout << buffer2d[j];
		//}
		//cout << endl <<endl;
		//fflush(nullptr);
	}
}

void MeshModel::updateTransform()
{
	mat4 trnsl_m = Translate(_trnsl.x, _trnsl.y, _trnsl.z);
	mat4 rot_m_x = RotateX(_rot.x);
	mat4 rot_m_y = RotateY(_rot.y);
	mat4 rot_m_z = RotateZ(_rot.z);
	mat4 scale_m = Scale(_scale.x, _scale.y, _scale.z);	


	_model_transform = scale_m * rot_m_z * rot_m_y * rot_m_z * trnsl_m; // TEST IF ORDER MATTERS

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