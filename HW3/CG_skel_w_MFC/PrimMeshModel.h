#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include "MeshModel.h"

class PrimMeshModel : public MeshModel
{

protected:
	PrimMeshModel(int num_vertex_raw, int num_faces, Renderer* rend)
	{
		this->renderer					= rend;
		this->num_vertices_raw			= num_vertex_raw;
		this->num_faces					= num_faces;
		
		vertex_positions_raw			= vector<vec3>(num_vertices_raw);
		vertex_faces_neighbors			= vector<vector<int>>(num_vertices_raw);
		faces_v_indices					= vector<int> (num_faces * 3);
		vertex_normals					= vector<vec3>(num_vertices_raw);
		GenerateMaterials();

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
	Cube(Renderer* rend) : PrimMeshModel(8, 12, rend)
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

		 //Call these last:
		 CreateVertexVectorForGPU();
		 GenerateAllGPU_Stuff();
	}
};

class Pyramid : public PrimMeshModel
{
public:
	Pyramid(Renderer* rend) : PrimMeshModel(5, 6, rend)
	{
		name = "Pyramid";


		vertex_positions_raw =
		{
			vec3(-0.5, -0.5, -0.5), // base 0
			vec3(0.5, -0.5, -0.5),  // base 1
			vec3(0.5,  -0.5, 0.5),	// base 2
			vec3(-0.5,  -0.5, 0.5),	// base 3
			vec3(0, 0.5,  0)		// apex
		};

		/*# Vertices
			v 0.0 0.0 0.0   # Vertex 1 (base)
			v 1.0 0.0 0.0   # Vertex 2 (base)
			v 1.0 0.0 1.0   # Vertex 3 (base)
			v 0.0 0.0 1.0   # Vertex 4 (base)
			v 0.5 1.0 0.5   # Vertex 5 (apex)*/

		/*v 0.0 0.0 0.0   # Vertex 1 (base)
			v 1.0 0.0 0.0   # Vertex 2 (base)
			v 1.0 1.0 0.0   # Vertex 3 (base)
			v 0.0 1.0 0.0   # Vertex 4 (base)
			v 0.5 0.5 1.0   # Vertex 5 (apex)*/

		faces_v_indices =
		{
			0,4,3,	// 2
			
			0,4,1,	// 1

			1,4,2,	// 3

			2,4,3,	// 4

			0,1,2,	// Base
			0,3,2
		};


		/* Call these functions after setting the vertex & faces data */
		initNeighbors();
		initBoundingBox();
		calculateFaceNormals();
		estimateVertexNormals();

		//Call these last:
		CreateVertexVectorForGPU();
		GenerateAllGPU_Stuff();
	}
};

class TriPyramid : public PrimMeshModel
{
public:
	TriPyramid(Renderer* rend) : PrimMeshModel(4, 4, rend)
	{
		name = "Triangular Pyramid";


		vertex_positions_raw =
		{
			vec3(-0.5 , -0.5, -0.5),	// base 1
			vec3(0.5  , -0.5, -0.5),	// base 2
			vec3(0    , -0.5,  0.5),    // base 3 
			vec3(0    ,  0.5,  0)		// apex

		};

		faces_v_indices =
		{

			0,1,2,	// Base

			0,3,1,	// 1
			
			0,2,3,	// 2

			1,3,2,	// 3

		};


		/* Call these functions after setting the vertex & faces data */
		initNeighbors();
		initBoundingBox();
		calculateFaceNormals();
		estimateVertexNormals();

		//Call these last:
		CreateVertexVectorForGPU();
		GenerateAllGPU_Stuff();
	}
};


