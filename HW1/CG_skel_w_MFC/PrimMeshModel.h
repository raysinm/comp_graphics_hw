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
		int num_triangles = 12; // 6 square faces, 2 triangles each
		num_vertices = num_triangles*3;	// With repeats
		
		vertex_positions = new vec3[num_vertices]; /*BUG - fixed: each face is made of 3 vertecies.*/
		t_vertex_positions = new vec3[num_vertices];
												   //vertex_normals = new vec3[num_vertices];
		buffer2d = new vec2[num_vertices]; //Worst case: each vertex is on a different pixel
		
		 std::vector<vec3> v= { 
							vec3(-0.5, -0.5, 0.5), 
							vec3(0.5, -0.5, 0.5),
							vec3(-0.5,  0.5, 0.5),
							vec3(0.5,  0.5, 0.5),
							vec3(-0.5, -0.5,  -0.5),
							vec3(0.5, -0.5,  -0.5),
							vec3(-0.5,  0.5,  -0.5),
							vec3(0.5,  0.5,  -0.5)
		};

		 std::vector<GLint> indices = {
			 0,2,3,	// Front
			 0,1,3,

			 2,6,7,	// Top
			 2,3,7,

			 0,4,5, // Bottom
			 0,1,5,

			 0,2,6,	// Left
			 0,4,6,

			 1,3,7,	// Right
			 1,5,7,

			 4,6,7,	// Back
			 4,5,7
		 };


		for (int i = 0; i < num_vertices; i++)
		{
			vertex_positions[i] = vec3(v[indices[i]]);

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

class TriPyramid : public PrimMeshModel
{
public:
	TriPyramid()
	{
		name = "Triangular-based Pyramid";
		int num_triangles = 4; // 6 square faces, 2 triangles each
		num_vertices = num_triangles * 3;	// With repeats

		vertex_positions = new vec3[num_vertices]; /*BUG - fixed: each face is made of 3 vertecies.*/
		t_vertex_positions = new vec3[num_vertices];
		//vertex_normals = new vec3[num_vertices];
		buffer2d = new vec2[num_vertices]; //Worst case: each vertex is on a different pixel

		std::vector<vec3> v = {
						   vec3(0, 0.5, 0),
						   vec3(-0.5, -0.5, 0.5),
						   vec3(0.5,  -0.5, 0.5),
						   vec3(0.5,  -0.5, -0.5),
						   vec3(-0.5, -0.5,  -0.5)
};

		std::vector<GLint> indices = {
			0,1,2,	// Base

			0,2,3,	// 1
			
			0,3,4,	// 2

			0,4,1	// 3
		};


		for (int i = 0; i < num_vertices; i++)
		{
			vertex_positions[i] = vec3(v[indices[i]]);

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

