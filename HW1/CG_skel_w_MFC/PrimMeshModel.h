#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include "MeshModel.h"

class PrimMeshModel : public MeshModel
{

};

//class TriPyramid : PrimMeshModel	// triangular base
//{
//public:
//	TriPyramid::TriPyramid()	
//	{
//		name = "Triangular Pyramid";
//		
//		int num_faces = 4;
//		vertex_positions = new vec3[num_vertices]; /*BUG - fixed: each face is made of 3 vertecies.*/
//		vertex_normals = new vec3[num_vertices];
//		buffer2d = new vec2[num_vertices]; //Worst case: each vertex is on a different pixel
//
//		vertex_positions = {vec3(0,0,0), vec3()}
//	}
//};

class Cube : public PrimMeshModel
{
public:
	Cube() 
	{
		name = "Cube";

		const int num_faces = 12;	// 6 square faces, 2 triangles each
		num_vertices = 3 * num_faces;	// 6 square faces, 2 triangles each
		vertex_positions = new vec3[num_vertices]; /*BUG - fixed: each face is made of 3 vertecies.*/
		t_vertex_positions = new vec3[num_vertices];
												   //vertex_normals = new vec3[num_vertices];
		buffer2d = new vec2[num_vertices]; //Worst case: each vertex is on a different pixel
		
		vec3 temp[12*num_faces] =
		{
			vec3(0,0,0), vec3(1,0,0), vec3(0,1,0),
			vec3(0,0,0), vec3(0,1,0), vec3(0,0,-1),
			vec3(0,0,0), vec3(0,0,-1), vec3(1,0,0),
			vec3(0,0,0), vec3(0,1,-1), vec3(0,0,-1),
			vec3(1,1,0), vec3(0,1,0), vec3(1,0,0),
			vec3(1,1,0), vec3(0,1,0), vec3(0,1,-1),
			vec3(1,0,1), vec3(0,0,-1), vec3(1,0,0),
			vec3(1,1,1), vec3(0,1,-1), vec3(1,1,0),
			vec3(1,1,1), vec3(1,1,0), vec3(1,0,0),
			vec3(1,1,1), vec3(1,0,1), vec3(1,0,0),
			vec3(1,1,1), vec3(0,1,-1), vec3(0,0,-1),
			vec3(1,1,1), vec3(0,0,-1), vec3(1,0,1)
		};

		for (int i = 0; i < num_vertices; i++)
		{
			vertex_positions[i] = temp[i];
		}

		//cout << "CUBE: vertex_positions: ";
		//for (int j = 0; j <= num_vertices; j++)
		//{
		//	cout << vertex_positions[j];
		//}
		//cout << endl;
		//TODO: Add vertex normals
	}
};