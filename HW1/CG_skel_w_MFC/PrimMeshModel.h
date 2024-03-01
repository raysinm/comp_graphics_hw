#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include "MeshModel.h"

class PrimMeshModel : public MeshModel
{

protected:
	PrimMeshModel(int num_vertex_raw, int num_faces)
	{
		this->num_vertices_raw			= num_vertex_raw;
		this->num_faces					= num_faces;
		this->num_vertices				= num_faces * 3;
		
		vertex_positions_raw			= vector<vec3>(num_vertices_raw);
		t_vertex_positions_normalized	= vector<vec3>(num_vertices_raw);
		vertex_faces_neighbors			= vector<vector<int>>(num_vertices_raw);
		faces_v_indices					= vector<int> (num_faces * 3);
		vertex_normals					= vector<vec3>(num_vertices_raw);
		buffer_vertrices				= new Vertex[num_vertices];
		buffer2d_v_normals				= new vec2[num_vertices_raw * 2];
		buffer2d_f_normals				= new vec2[num_faces * 2];
	}

	void initNeighbors()
	{
		//iterate through all stored faces and store neighbors normal vectors (for each vertex)
		for (unsigned int face_id = 0; face_id < num_faces; face_id++)
			for (unsigned int i = 0; i < 3; i++)
				vertex_faces_neighbors[faces_v_indices[(face_id * 3) + i]].push_back(face_id);
	}
};

class Cube : public PrimMeshModel
{
public:
	Cube() : PrimMeshModel(8, 12)
	{
		name = "Cube";
		
		 vertex_positions_raw =
		 { 
			vec3(-0.5, -0.5, 0.5), 
			vec3(0.5, -0.5, 0.5),
			vec3(-0.5,  0.5, 0.5),
			vec3(0.5,  0.5, 0.5),

			vec3(-0.5, -0.5,  -0.5),
			vec3(0.5, -0.5,  -0.5),
			vec3(-0.5,  0.5,  -0.5),
			vec3(0.5,  0.5,  -0.5)
		};

		 faces_v_indices =
		 {
			 0,3,2,	// Front
			 0,1,3,

			 2,7,6,	// Top
			 2,3,7,

			 0,4,5, // Bottom
			 0,5,1,

			 0,2,6,	// Left
			 0,6,4,

			 1,7,3,	// Right
			 1,5,7,

			 4,6,7,	// Back
			 4,7,5
		 };


		
		 
		 /* Call these functions after setting the vertex & faces data */
		 initNeighbors();
		 initBoundingBox();
		 calculateFaceNormals();
		 estimateVertexNormals();
	}
};

class TriPyramid : public PrimMeshModel
{
public:
	TriPyramid() : PrimMeshModel(5, 4)
	{
		name = "Pyramid";


		vertex_positions_raw =
		{
			vec3(0, 0.5, 0),
			vec3(-0.5, -0.5, 0.5),
			vec3(0.5,  -0.5, 0.5),
			vec3(0.5,  -0.5, -0.5),
			vec3(-0.5, -0.5,  -0.5)
		};

		faces_v_indices =
		{
			0,1,2,	// Base

			0,2,3,	// 1
			
			0,3,4,	// 2

			0,4,1	// 3
		};


		/* Call these functions after setting the vertex & faces data */
		initNeighbors();
		initBoundingBox();
		calculateFaceNormals();
		estimateVertexNormals();
	}
};

