//#pragma once
//#include "vec.h"
//#include "mat.h"
//#include <string>
//#include <math.h>
//#define GRID_SIZE 10
//
//
//class Grid
//{
//private:
//	float inner_w;
//	unsigned int grid_size, num_vertices;
//	vector<vec3> vertices;
//	vec2* buffer;
//	friend class Scene;
//	//rgb(164, 0, 179)
//public:
//	Grid(float inner_w = 1.0, int grid_size = GRID_SIZE) : inner_w(inner_w), grid_size(grid_size), 
//														   num_vertices((int)ceil(grid_size / inner_w * 4)),
//														   vertices(num_vertices)
//	{
//		buffer = new vec2(num_vertices);
//		
//		// Forming middle cross that goes through 0,0
//		// NOTICE Every 2 vertices is a line
//		vertices.push_back(vec3(0, 0, -grid_size));
//		vertices.push_back(vec3(0, 0, grid_size));
//
//		vertices.push_back(vec3(-grid_size, 0, 0));
//		vertices.push_back(vec3(grid_size, 0, 0));
//
//		for (float x = inner_w; x < grid_size; x += inner_w)
//		{
//			for (float z = inner_w; z < grid_size; z += inner_w)
//			{
//				// Add 2 horizontal and 2 vertical lines starting from 0,0
//				// 2 vertices per line: 8 vertices total
//				vertices.push_back(vec3(-x, 0, -grid_size));
//				vertices.push_back(vec3(-x, 0, grid_size));
//				
//				vertices.push_back(vec3(x, 0, -grid_size));
//				vertices.push_back(vec3(x, 0, grid_size));
//
//				vertices.push_back(vec3(-grid_size, 0, -z));
//				vertices.push_back(vec3(grid_size, 0, -z));
//				
//				vertices.push_back(vec3(-grid_size, 0, z));
//				vertices.push_back(vec3(grid_size, 0, z));
//
//			}
//		}
//
//	}
//	void draw(mat4& cTransform, mat4& projection)
//	{
//		mat3 cTransform_rot = TopLeft3(cTransform);
//		mat3 cTransform_rot_inv = transpose(cTransform_rot);
//		vec3 cTransfrom_trnsl = RightMostVec(cTransform);
//		mat4 cTransform_inv(cTransform_rot_inv, -(cTransform_rot_inv * cTransfrom_trnsl));
//
//		int i = 0;
//		for (auto v : vertices)
//		{
//			vec4 vi(v);
//			vi = projection * (cTransform_inv * vi);
//			/*cout << "v: " << v << endl;
//			cout << "vi: " << vi << endl;*/
//			buffer[i] = vec2(vi.x, vi.y);
//			i++;
//		}
//	}
//
//	vec2* Get2dBuffer() { return buffer; };
//	unsigned int Get2dBuffer_len() { return num_vertices; };
//
//};