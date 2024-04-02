#include "stdafx.h"
#include "scene.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <random>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct FaceIdcs
{
	int v[4];   //vertex index
	int vn[4];  //vertex index from vertex normals array
	int vt[4];  //vertex texture

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

MeshModel::MeshModel(string fileName, Renderer* rend) : MeshModel(rend)
{
	loadFile(fileName);
	initBoundingBox();
	GenerateMaterials();
	CreateVertexVectorForGPU();
	GenerateAllGPU_Stuff();
}

MeshModel::~MeshModel(void)
{
	if (textureLoaded && tex > 0)
	{
		glBindTexture(GL_TEXTURE_2D, tex);
		glDeleteTextures(1, &tex);
	}

	if (normalMapLoaded && nmap > 0)
	{
		glBindTexture(GL_TEXTURE_2D, nmap);
		glDeleteTextures(1, &nmap);
	}

	if (marbletex > 0)
	{
		glBindTexture(GL_TEXTURE_2D, marbletex);
		glDeleteTextures(1, &marbletex);
	}
}

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

vec2* MeshModel::GetBuffer(MODEL_OBJECT obj)
{

	return nullptr;
}

void MeshModel::SetCanonicalTextureCoordsToPlaneProjection()
{
	float w = max_x - min_x;
	float h = max_y - min_y;
	verticesTextures_canonical_gpu.clear();

	for (auto vertexIndex : faces_v_indices)
	{
		vec3 vPos = vertex_positions_raw[vertexIndex];
		
		//Project the vertex on to the XY-Plane
		vec2 st = vec2(vPos.x, vPos.y);	
		
		//Normalize the st coordinate to [0, 1]
		st.x = (st.x - min_x) / w;
		st.y = (st.y - min_y) / h;
		
		//Add the st coordinate to the vector
		verticesTextures_canonical_gpu.push_back(st);
	}
}

void MeshModel::SetCanonicalTextureCoordsToSphereProjection()
{

	verticesTextures_canonical_gpu.clear();

	vec3 centerOfSphere = vec3(0, 0, 0);
	float radius = 0;
	for (auto vertexIndex : faces_v_indices)
	{
		vec3 v = vertex_positions_raw[vertexIndex];
		centerOfSphere += v;
		radius = max(length(v), radius);
	}
	centerOfSphere /= faces_v_indices.size();

	for (auto vertexIndex : faces_v_indices)
	{
		vec3 c = centerOfSphere;
		vec3 v = vertex_positions_raw[vertexIndex];

		
		float theta = atan2(-(v.z - c.z), v.x - c.x);
		float s = (theta + M_PI) / 2 * M_PI;

		float phi = acos(-(v.y - c.y) / radius);
		float t = phi / M_PI;

		//Add the st coordinate to the vector
		verticesTextures_canonical_gpu.push_back(vec2(s, t));
	}
}

unsigned int MeshModel::GetBuffer_len(MODEL_OBJECT obj)
{
	switch (obj)
	{
	case MODEL_WIREFRAME:
		return vertex_positions_wireframe_gpu.size();
	case MODEL_TRIANGLES:
		return vertex_positions_triangle_gpu.size();
	case BBOX:
		return vertex_positions_bbox_gpu.size();
	case V_NORMAL:
		return vertex_positions_Vnormals_gpu.size();
	case F_NORMAL:
		return vertex_positions_Fnormals_gpu.size();

	default:
		return -1;
	}
}

MeshModel::MeshModel(Renderer* rend)
{
	this->renderer = rend;
	ResetAllUserTransforms();
	start_time = chrono::high_resolution_clock::now();


}

void MeshModel::GenerateVBO_WireFrame()
{
	glBindVertexArray(VAOs[VAO_VERTEX_WIREFRAME]);
	glGenBuffers(1, VBOs[VAO_VERTEX_WIREFRAME]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_WIREFRAME][VBO_VERTEX_POS]);


	//Multiply len by 3 because each vertex is holding 3-floats
	int lenInBytes = vertex_positions_wireframe_gpu.size() * 3 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_positions_wireframe_gpu.data(), GL_STATIC_DRAW);

	GLint vPosition = glGetAttribLocation(renderer->program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	glBindVertexArray(0);
}

void MeshModel::GenerateVBO_Triangles()
{
	glBindVertexArray(VAOs[VAO_VERTEX_TRIANGLE]);
	glGenBuffers(VBO_COUNT, VBOs[VAO_VERTEX_TRIANGLE]);

	/* vPosition */
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_POS]);
		int lenInBytes = vertex_positions_triangle_gpu.size() * 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_positions_triangle_gpu.data(), GL_STATIC_DRAW);
		GLint vPosition = glGetAttribLocation(renderer->program, "vPosition");
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vPosition);
	}

	/* fPosition */
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_FACE_POS]);
		int lenInBytes = vertex_face_positions_triangle_gpu.size() * 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_face_positions_triangle_gpu.data(), GL_STATIC_DRAW);
		GLint fPosition = glGetAttribLocation(renderer->program, "fPosition");
		glVertexAttribPointer(fPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(fPosition);
	}

	/* vn */
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_VN]);
		int lenInBytes = vertex_vn_triangle_gpu.size() * 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_vn_triangle_gpu.data(), GL_STATIC_DRAW);
		GLint vn = glGetAttribLocation(renderer->program, "vn");
		glVertexAttribPointer(vn, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vn);
	}
	
	/* fn */
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_FN]);
		int lenInBytes = vertex_fn_triangle_gpu.size() * 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_fn_triangle_gpu.data(), GL_STATIC_DRAW);
		GLint fn = glGetAttribLocation(renderer->program, "fn");
		glVertexAttribPointer(fn, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(fn);
	}

	/* DIFFUSE_COLOR_FLAT */
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_DIFFUSE_COLOR_FLAT]);
		int lenInBytes = vertex_diffuse_color_flat_triangle_gpu.size() * 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_diffuse_color_flat_triangle_gpu.data(), GL_STATIC_DRAW);
		GLint non_uniformColor_diffuse_FLAT = glGetAttribLocation(renderer->program, "non_uniformColor_diffuse_FLAT");
		glVertexAttribPointer(non_uniformColor_diffuse_FLAT, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(non_uniformColor_diffuse_FLAT);
	}

	/* DIFFUSE_COLOR_FLAT */
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_DIFFUSE_COLOR]);
		int lenInBytes = vertex_diffuse_color_triangle_gpu.size() * 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_diffuse_color_triangle_gpu.data(), GL_STATIC_DRAW);
		GLint non_uniformColor_diffuse = glGetAttribLocation(renderer->program, "non_uniformColor_diffuse");
		glVertexAttribPointer(non_uniformColor_diffuse, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(non_uniformColor_diffuse);
	}

	/* TEXTURE_MAP */
	{
		if (verticesTextures_original_gpu.size() > 0)
		{
			UpdateTextureCoordsInGPU();
		}
	}

	UpdateTangentSpaceInGPU();

	glBindVertexArray(0);
}

void MeshModel::UpdateTangentSpaceInGPU()
{
	/* TANGENT */
	if (verticesTextures_original_gpu.size() > 0)
	{

		glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_FACE_TANGENT]);
		int lenInBytes = triangles_TangentV_gpu.size() * 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, lenInBytes, triangles_TangentV_gpu.data(), GL_STATIC_DRAW);
		GLint tangent = glGetAttribLocation(renderer->program, "tangent");
		glVertexAttribPointer(tangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(tangent);
	}

}

void MeshModel::UpdateTextureCoordsInGPU()
{
	vector<vec2>* dataToUse = nullptr;

	if (textureMode == TEXTURE_FROM_FILE)
	{
		dataToUse = &verticesTextures_original_gpu;
	}
	else if (textureMode == TEXTURE_CANONICAL_1)
	{
		SetCanonicalTextureCoordsToPlaneProjection();
		dataToUse = &verticesTextures_canonical_gpu;
	}
	else if (textureMode == TEXTURE_CANONICAL_2)
	{
		SetCanonicalTextureCoordsToSphereProjection();
		dataToUse = &verticesTextures_canonical_gpu;
	}
	if (dataToUse->size() == 0)
		return;

	glUseProgram(renderer->program);
	glBindVertexArray(VAOs[VAO_VERTEX_TRIANGLE]);

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_TEXTURE_MAP]);
	int lenInBytes = dataToUse->size() * sizeof((*dataToUse)[0]);
	glBufferData(GL_ARRAY_BUFFER, lenInBytes, dataToUse->data(), GL_STATIC_DRAW);
	GLint texcoord = glGetAttribLocation(renderer->program, "texcoord");
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texcoord);

	calculateTangentSpace();
	UpdateTangentSpaceInGPU();
}

void MeshModel::GenerateVBO_BBox()
{
	glBindVertexArray(VAOs[VAO_VERTEX_BBOX]);
	glGenBuffers(1, VBOs[VAO_VERTEX_BBOX]);

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_BBOX][VBO_VERTEX_POS]);
	//Multiply len by 3 because each vertex is holding 3-floats
	int lenInBytes = vertex_positions_bbox_gpu.size() * 3 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_positions_bbox_gpu.data(), GL_STATIC_DRAW);

	GLint vPosition = glGetAttribLocation(renderer->program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	glBindVertexArray(0);
}

void MeshModel::GenerateVBO_vNormals()
{
	glBindVertexArray(VAOs[VAO_VERTEX_VNORMAL]);
	glGenBuffers(2, VBOs[VAO_VERTEX_VNORMAL]);

	/* Positions */
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_VNORMAL][VBO_VERTEX_POS]);
	int lenInBytes = vertex_positions_Vnormals_gpu.size() * 3 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_positions_Vnormals_gpu.data(), GL_STATIC_DRAW);
	GLint vPosition = glGetAttribLocation(renderer->program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	/* Directions */
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_VNORMAL][1]);
	lenInBytes = vertex_directions_Vnormals_gpu.size() * 3 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_directions_Vnormals_gpu.data(), GL_STATIC_DRAW);
	GLint vn = glGetAttribLocation(renderer->program, "vn");
	glVertexAttribPointer(vn, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vn);


	glBindVertexArray(0);
}

void MeshModel::GenerateVBO_fNormals()
{
	glBindVertexArray(VAOs[VAO_VERTEX_FNORMAL]);
	glGenBuffers(2, VBOs[VAO_VERTEX_FNORMAL]);

	/* Positions */
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_FNORMAL][VBO_VERTEX_POS]);
	int lenInBytes = vertex_positions_Fnormals_gpu.size() * 3 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_positions_Fnormals_gpu.data(), GL_STATIC_DRAW);
	GLint vPosition = glGetAttribLocation(renderer->program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	/* Directions */
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_FNORMAL][1]);
	lenInBytes = vertex_directions_Fnormals_gpu.size() * 3 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_directions_Fnormals_gpu.data(), GL_STATIC_DRAW);
	GLint fn = glGetAttribLocation(renderer->program, "fn");
	glVertexAttribPointer(fn, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(fn);


	glBindVertexArray(0);
}

void MeshModel::GenerateAllGPU_Stuff()
{	
	//Genereate VAO and VBOs in GPU:
	glGenVertexArrays(VAO_COUNT, this->VAOs);

	GenerateVBO_WireFrame();
	GenerateVBO_Triangles();
	GenerateVBO_BBox();
	GenerateVBO_fNormals();
	GenerateVBO_vNormals();
}

vector<vec3> MeshModel::duplicateEachElement(const vector<vec3>& v, const int duplicateNumber)
{
	vector<vec3> temp;
	for (auto elemnt : v)
		for (int i = 0; i < duplicateNumber; i++)
			temp.push_back(elemnt);
		
	return temp;
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> verticesNormals;
	vector<vec2> verticesTextures_raw;

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
		else if (lineType == "vt") //Vertex Texture
		{
			verticesTextures_raw.push_back(vec2fFromStream(issLine));
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

	vertex_normals					= vector<vec3> (num_vertices_raw);
	vertex_faces_neighbors			= vector<vector<int>> (num_vertices_raw);

	bool v_normals_exist = verticesNormals.size() > 0;

	//iterate through all stored faces and create triangles
	unsigned int face_id = 0;
	for (auto face : faces)
	{
		for (int i = 0; i < 3; i++)
		{
			int vertIndex = face.v[i] - 1;
			int vertIndex_Texture = face.vt[i] - 1;
			faces_v_indices.push_back(vertIndex);
			vertex_faces_neighbors[vertIndex].push_back(face_id);
			if(verticesTextures_raw.size() > 0)
				verticesTextures_original_gpu.push_back(verticesTextures_raw[vertIndex_Texture]);
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

void MeshModel::loadTextureFromFile()
{
	CFileDialog dlg(TRUE, _T(".png"), NULL, NULL, _T("(*.png)|*.png|All Files (*.*)|*.*||"));
	if (dlg.DoModal() == IDOK)
	{
		std::string filePath((LPCTSTR)dlg.GetPathName());
		stbi_set_flip_vertically_on_load(true);
		textureMap.image_data = stbi_load(filePath.c_str(), &textureMap.width, &textureMap.height, &textureMap.channels, 0);
		GenerateTexture();
		textureLoaded = true;
	}
}

void MeshModel::loadNMapFromFile()
{
	CFileDialog dlg(TRUE, _T(".png"), NULL, NULL, _T("(*.png)|*.png|All Files (*.*)|*.*||"));
	if (dlg.DoModal() == IDOK)
	{
		std::string filePath((LPCTSTR)dlg.GetPathName());
		stbi_set_flip_vertically_on_load(true);
		normalMap.image_data = stbi_load(filePath.c_str(), &normalMap.width, &normalMap.height, &normalMap.channels, 0);
		GenerateNMap();
		normalMapLoaded = true;
	}

}

void MeshModel::initBoundingBox()
{
	if (vertex_positions_raw.empty()) return;
	
	// Bounding box init
	vertex_positions_bbox_gpu = vector<vec3> (num_bbox_vertices);


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
			0,1,	// Top
			2,3,
			1,2,
			0,3,

			4,5,	// Bottom
			5,6,
			4,7, 
			6,7,

			1,5,	// Left
			0,4,
			
			3,7,	// Right
			2,6

			//0,4,7,	// Front
			//0,3,7,

			//1,2,6,	// Back
			//1,5,6
	};

	for (unsigned int i = 0; i < num_bbox_vertices; i++)
		vertex_positions_bbox_gpu[i] = v[indices[i]];
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

void MeshModel::CreateVertexVectorForGPU()
{
	vector<vec3> tmpFacePositions;
	vector<vec3> tmpFaceDirections;

	/* Generate positions vector */
	UINT k = 0;
	for (UINT f = 0; f < num_faces; f++)
	{
		for (UINT i = 0; i < 3; i++)
		{
			UINT vertIndex = faces_v_indices[k++];
			vertex_positions_triangle_gpu.push_back(vertex_positions_raw[vertIndex]);
			vertex_vn_triangle_gpu.push_back(vertex_normals[vertIndex]);
		}
		
		//Add center of face:
		vec3 v_i = vertex_positions_raw[faces_v_indices[(f * 3) + 0]];
		vec3 v_j = vertex_positions_raw[faces_v_indices[(f * 3) + 1]];
		vec3 v_k = vertex_positions_raw[faces_v_indices[(f * 3) + 2]];
		tmpFacePositions.push_back((v_i + v_j + v_k) / 3);
		tmpFaceDirections.push_back(normalize(cross(v_j - v_i, v_k - v_i)));

		//Add 3 lines for wireframe:
		int vertIndex_A = faces_v_indices[(f * 3) + 0];
		int vertIndex_B = faces_v_indices[(f * 3) + 1];
		int vertIndex_C = faces_v_indices[(f * 3) + 2];


		/* A --> B*/
		vertex_positions_wireframe_gpu.push_back(vertex_positions_raw[vertIndex_A]);
		vertex_positions_wireframe_gpu.push_back(vertex_positions_raw[vertIndex_B]);

		/* A --> C*/
		vertex_positions_wireframe_gpu.push_back(vertex_positions_raw[vertIndex_A]);
		vertex_positions_wireframe_gpu.push_back(vertex_positions_raw[vertIndex_C]);

		/* B --> C*/
		vertex_positions_wireframe_gpu.push_back(vertex_positions_raw[vertIndex_B]);
		vertex_positions_wireframe_gpu.push_back(vertex_positions_raw[vertIndex_C]);
	}
	
	/* Generate vNormal vector */
	vertex_positions_Vnormals_gpu  = duplicateEachElement(vertex_positions_raw);
	vertex_directions_Vnormals_gpu = duplicateEachElement(vertex_normals);

	/* Generate fNormal vector */
	vertex_positions_Fnormals_gpu  = duplicateEachElement(tmpFacePositions);
	vertex_directions_Fnormals_gpu = duplicateEachElement(face_normals);

	/* Generate face positions and directions vector */
	vertex_face_positions_triangle_gpu	= duplicateEachElement(tmpFacePositions , 3);
	vertex_fn_triangle_gpu				= duplicateEachElement(tmpFaceDirections, 3);

	PopulateNonUniformColorVectorForGPU();
	if(verticesTextures_original_gpu.size() > 0)
		calculateTangentSpace();
	generateMarbleTexture();

}

void MeshModel::PopulateNonUniformColorVectorForGPU()
{
	vertex_diffuse_color_flat_triangle_gpu.clear();
	vertex_diffuse_color_triangle_gpu.clear();
	
	UINT k = 0;

	for (UINT f = 0; f < num_faces; f++)
	{
		vec3 avgColorOfFace = vec3(0);
		for (UINT i = 0; i < 3; i++)
		{
			UINT vertIndex = faces_v_indices[k++];
			vec3 color = materials[vertIndex].c_diffuse;
			vertex_diffuse_color_triangle_gpu.push_back(color);
			avgColorOfFace += color;
		}
		avgColorOfFace /= 3;

		for (UINT i = 0; i < 3; i++)
			vertex_diffuse_color_flat_triangle_gpu.push_back(avgColorOfFace);
	}
}

void MeshModel::draw(mat4& cTransform, mat4& projection, bool allowClipping, mat4& cameraRot)
{
	//if (!userInitFinished) //Dont start to draw before user clicked 'OK' in the popup window...
	//	return;

	//face_normals_viewspace.clear();
	//vertex_normals_viewspace.clear();

	////updateTransform();
	////updateTransformWorld();

	////TODO:
	////

	////Apply all transformations and save in t_vertex_positions_normalized array
	//for (unsigned int i = 0; i < vertex_positions_raw.size(); i++)
	//{
	//	vec4 v_i(vertex_positions_raw[i]);

	//	//Apply model-view transformation
	//	v_i = (_world_transform * (_model_transform * v_i));

	//	//Apply camera transform matrix
	//	v_i = cTransform * v_i;

	//	//Save vertex in camera space for shading algorithms
	//	t_vertex_positions_cameraspace[i] = vec3(v_i.x, v_i.y, v_i.z);


	//	//Apply projection:
	//	v_i = projection * v_i;

	//	// Save result in clip space
	//	t_vertex_positions_normalized[i] = vec3(v_i.x, v_i.y, v_i.z) / v_i.w;
	//}

	//// Clipping

	///* add the 3 points of the current face: */
	//if (atleast_one_vertex_in_bound || !allowClipping)
	//{
	//	for (unsigned int v = 0; v < 3; v++)
	//	{
	//		UINT vertIndex = faces_v_indices[(face_indx * 3) + v];
	//		vec3 point = t_vertex_positions_normalized[vertIndex];
	//		vec3 point_cameraspace = t_vertex_positions_cameraspace[vertIndex];
	//		buffer_vertrices[(buffer_i * 3) + v] = Vertex(point, vertIndex, face_indx, point_cameraspace);
	//		num_vertices_to_draw++;
	//	}
	//	buffer_i++;
	//}

	//// Bounding box buffer
	//if (showBoundingBox)
	//{
	//	for (unsigned int j = 0; j < num_bbox_vertices; j++)
	//	{
	//		vec4 v_j(vertex_positions_bbox_gpu[j]);

	//		//Apply model-view transformations:
	//		v_j = cTransform * (_world_transform * (_model_transform * v_j));

	//		//Project:
	//		v_j = projection * v_j;

	//		//Add to 2d buffer: 
	//		buffer2d_bbox[j] = vec2(v_j.x, v_j.y) / v_j.w;
	//	}
	//}

	//// Vertex normals buffer
	//{
	//	for (unsigned int j = 0; j < num_vertices_raw; j++)
	//	{
	//		vec4 v_j(vertex_normals[j]);

	//		//Transform the normal vector:
	//		v_j = cameraRot * (_world_transform_for_normals * (_model_transform_for_normals * v_j));

	//		//Add it before projection
	//		vec4 v_j_J = normalize(v_j);
	//		vertex_normals_viewspace.push_back(vec3(v_j_J.x, v_j_J.y, v_j_J.z));

	//		if (showVertexNormals)
	//		{
	//			//Project the vector:
	//			v_j = projection * v_j;

	//			//Make sure it is still normalized:
	//			v_j = normalize(v_j);

	//			vec4 start_point = t_vertex_positions_normalized[j];
	//			vec4 end_point = start_point + (v_j * length_vertex_normals);

	//			buffer2d_v_normals[j * 2 + 0] = vec2(start_point.x, start_point.y);
	//			buffer2d_v_normals[j * 2 + 1] = vec2(end_point.x, end_point.y);
	//		}
	//	}
	//}

	//// Face normals buffer
	//{
	//	unsigned int buffer_i = 0;
	//	for (unsigned int face_indx = 0; face_indx < num_faces; face_indx++)
	//	{
	//		vec4 v_n(face_normals[face_indx]);

	//		//Transform the normal vector:
	//		v_n = cameraRot * (_world_transform_for_normals * (_model_transform_for_normals * v_n));

	//		//Add it before projection
	//		vec4 v_n_N = normalize(v_n);
	//		face_normals_viewspace.push_back(vec3(v_n_N.x, v_n_N.y, v_n_N.z));

	//		// Face normals buffer
	//		if (showFaceNormals)
	//		{
	//			//Project the vector:
	//			v_n = projection * v_n;

	//			//Make sure it is still normalized:
	//			v_n = normalize(v_n);


	//			// Cacluate center of face as the start point:
	//			vec3 v0 = t_vertex_positions_normalized[faces_v_indices[(face_indx * 3) + 0]];
	//			vec3 v1 = t_vertex_positions_normalized[faces_v_indices[(face_indx * 3) + 1]];
	//			vec3 v2 = t_vertex_positions_normalized[faces_v_indices[(face_indx * 3) + 2]];

	//			vec3 start_point = vec3(v0 + v1 + v2) / 3;
	//			vec3 end_point = start_point + (vec3(v_n.x, v_n.y, v_n.z) * length_face_normals);

	//			buffer2d_f_normals[(buffer_i * 2) + 0] = vec2(start_point.x, start_point.y);
	//			buffer2d_f_normals[(buffer_i * 2) + 1] = vec2(end_point.x, end_point.y);
	//			++buffer_i;
	//		}
	//	}
	//}
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
	_model_transform_for_normals = scale_inverse_m * (rot_m_z * (rot_m_y * rot_m_x));
	
}

void MeshModel::updateTransformWorld()
{
	mat4 rot_m_x = RotateX(_rot_w.x);
	mat4 rot_m_y = RotateY(_rot_w.y);
	mat4 rot_m_z = RotateZ(_rot_w.z);
	mat4 trnsl_m = Translate(_trnsl_w.x, _trnsl_w.y, _trnsl_w.z);
	mat4 scale_m = Scale(_scale_w.x, _scale_w.y, _scale_w.z);
	mat4 scale_inverse_m = Scale(1/_scale_w.x, 1/_scale_w.y, 1/_scale_w.z);
	
	_world_transform = scale_m * (rot_m_z * (rot_m_y * (rot_m_x * trnsl_m)));
	_world_transform_for_normals = scale_inverse_m * (rot_m_z * (rot_m_y * rot_m_x));
	
}

vec4 MeshModel::getCenterOffMass()
{
	vec4 res = vec4(0,0,0,1);
	res = _world_transform * (_model_transform * res);
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

void MeshModel::GenerateMaterials()
{
	materials.clear();
	
	for (int i = 0; i < num_vertices_raw; i++)
	{
		Material current = Material();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);

		float randomValue = dis(gen);
		float randomValue2 = dis(gen);
		float randomValue3 = dis(gen);
		
		current.c_diffuse = vec3(randomValue, randomValue2, randomValue3);

		materials.push_back(current);
	}

	nonUniformDataUpdated = false;
}

void MeshModel::UpdateModelViewInGPU(mat4& Tc, mat4& Tc_for_normals)
{
	// Calculate model view matrix:
	mat4 model = _world_transform * _model_transform;
	mat4 view = Tc;
	//model_view_mat = view * model; //computed in GPU

	// Calculate model view matrix:
	mat4 model_normals = _world_transform_for_normals * _model_transform_for_normals;
	mat4 view_normals = Tc_for_normals;
	//model_view_mat_for_normals = view_normals * model_normals; //computed in GPU


	/* Bind the model matrix*/
	glUniformMatrix4fv(glGetUniformLocation(renderer->program, "model"), 1, GL_TRUE, &(model[0][0]));

	/* Bind the view matrix*/
	glUniformMatrix4fv(glGetUniformLocation(renderer->program, "view"), 1, GL_TRUE, &(view[0][0]));
	
	/* Bind the model_normals matrix*/
	glUniformMatrix4fv(glGetUniformLocation(renderer->program, "model_normals"), 1, GL_TRUE, &(model_normals[0][0]));

	/* Bind the view_normals matrix*/
	glUniformMatrix4fv(glGetUniformLocation(renderer->program, "view_normals"), 1, GL_TRUE, &(view_normals[0][0]));


}

void MeshModel::UpdateMaterialinGPU()
{
	/* Bind the uniform colors */
	glUniform3fv(glGetUniformLocation(renderer->program, "uniformColor_emissive"), 1, &userDefinedMaterial.c_emissive[0]);
	glUniform3fv(glGetUniformLocation(renderer->program, "uniformColor_diffuse") , 1, &userDefinedMaterial.c_diffuse[0]);
	glUniform3fv(glGetUniformLocation(renderer->program, "uniformColor_specular"), 1, &userDefinedMaterial.c_specular[0]);
	
	/* Bind the Ka/Kd/Ks */
	glUniform1f(glGetUniformLocation(renderer->program, "Ka"), userDefinedMaterial.Ka);
	glUniform1f(glGetUniformLocation(renderer->program, "Kd"), userDefinedMaterial.Kd);
	glUniform1f(glGetUniformLocation(renderer->program, "Ks"), userDefinedMaterial.Ks);
	
	/* Bind the EmissiveFactor */
	glUniform1f(glGetUniformLocation(renderer->program, "EmissiveFactor"), userDefinedMaterial.EmissiveFactor);	

	/* Bind the COS_ALPHA */
	glUniform1i(glGetUniformLocation(renderer->program, "COS_ALPHA"), userDefinedMaterial.COS_ALPHA);

	/* Bind the isUniformMaterial */
	glUniform1i(glGetUniformLocation(renderer->program, "isUniformMaterial"), (int)isUniformMaterial);

	if (!isUniformMaterial && !nonUniformDataUpdated)
	{
		glBindVertexArray(VAOs[VAO_VERTEX_TRIANGLE]);

		nonUniformDataUpdated = true;

		PopulateNonUniformColorVectorForGPU();

		/* DIFFUSE_COLOR_FLAT */
		{
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_DIFFUSE_COLOR_FLAT]);
			int lenInBytes = vertex_diffuse_color_flat_triangle_gpu.size() * 3 * sizeof(float);
			glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_diffuse_color_flat_triangle_gpu.data(), GL_STATIC_DRAW);
			GLint non_uniformColor_diffuse_FLAT = glGetAttribLocation(renderer->program, "non_uniformColor_diffuse_FLAT");
			glVertexAttribPointer(non_uniformColor_diffuse_FLAT, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(non_uniformColor_diffuse_FLAT);
		}

		/* DIFFUSE_COLOR_FLAT */
		{
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[VAO_VERTEX_TRIANGLE][VBO_VERTEX_DIFFUSE_COLOR]);
			int lenInBytes = vertex_diffuse_color_triangle_gpu.size() * 3 * sizeof(float);
			glBufferData(GL_ARRAY_BUFFER, lenInBytes, vertex_diffuse_color_triangle_gpu.data(), GL_STATIC_DRAW);
			GLint non_uniformColor_diffuse = glGetAttribLocation(renderer->program, "non_uniformColor_diffuse");
			glVertexAttribPointer(non_uniformColor_diffuse, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(non_uniformColor_diffuse);
		}

		glBindVertexArray(0);
	}
}

void MeshModel::UpdateTextureInGPU()
{
	/* Bind the TextureMap enable / disable */
	glUniform1i(glGetUniformLocation(renderer->program, "usingTexture"), (int)useTexture);
	glUniform1i(glGetUniformLocation(renderer->program, "usingNormalMap"), (int)useNormalMap);
	glUniform1i(glGetUniformLocation(renderer->program, "usingMarbleTex"), (int)useProceduralTex);
	
	glUniform1i(glGetUniformLocation(renderer->program, "tex")		, 0);	//GL_TEXTURE0
	glUniform1i(glGetUniformLocation(renderer->program, "normalMap"), 1);	//GL_TEXTURE1
	glUniform1i(glGetUniformLocation(renderer->program, "texMarble"), 2);	//GL_TEXTURE2
	glUniform1i(glGetUniformLocation(renderer->program, "skybox")	, 3);	//GL_TEXTURE3
	
	glUniform1f(glGetUniformLocation(renderer->program, "minX"), min_x);
	glUniform1f(glGetUniformLocation(renderer->program, "maxX"), max_x);
	glUniform1f(glGetUniformLocation(renderer->program, "minY"), min_y);
	glUniform1f(glGetUniformLocation(renderer->program, "maxY"), max_y);
	glUniform1f(glGetUniformLocation(renderer->program, "minZ"), min_z);
	glUniform1f(glGetUniformLocation(renderer->program, "maxZ"), max_z);

	glUniform1f(glGetUniformLocation(renderer->program, "veinFreq"), vein_freq);
	glUniform1i(glGetUniformLocation(renderer->program, "veinThickness"), vein_thickness);
	glUniform1f(glGetUniformLocation(renderer->program, "colMixFactor"), mix_factor);
	glUniform1f(glGetUniformLocation(renderer->program, "noiseFreq"), noise_freq);
	glUniform1i(glGetUniformLocation(renderer->program, "noiseOctaves"), noise_octaves);
	glUniform1f(glGetUniformLocation(renderer->program, "noiseAmplitude"), noise_amplitude);




	if (useTexture && tex > 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	if (useNormalMap && nmap > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, nmap);
	}
	if (useProceduralTex && marbletex > 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, marbletex);

		glUniform3f(glGetUniformLocation(renderer->program, "mcolor1"), mcolor1.x, mcolor1.y, mcolor1.z);
		glUniform3f(glGetUniformLocation(renderer->program, "mcolor2"), mcolor2.x, mcolor2.y, mcolor2.z);

	}
}

void MeshModel::UpdateAnimationInGPU()
{
	if (colorAnimationType != 0 || vertexAnimationEnable)
	{
		auto end_time = chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
		float durationInSecondsFloat = std::chrono::duration<float>(duration).count();

		float time = (float)((float)durationInSecondsFloat - (int)durationInSecondsFloat);
		float smoothTime = 0.5 + (sinf(M_PI*(time-0.5f))) / 2;

		glUniform1f(glGetUniformLocation(renderer->program, "time"), time);
		glUniform1f(glGetUniformLocation(renderer->program, "smoothTime"), smoothTime);
		glUniform1f(glGetUniformLocation(renderer->program, "minX"), min_x);
		glUniform1f(glGetUniformLocation(renderer->program, "maxX"), max_x);

	}
	glUniform1i(glGetUniformLocation(renderer->program, "colorAnimateType"), colorAnimationType);
	glUniform1i(glGetUniformLocation(renderer->program, "vertexAnimationEnable"), (int)vertexAnimationEnable);
}

void MeshModel::calculateTangentSpace()
{
	if (textureMode == TEXTURE_FROM_FILE && verticesTextures_original_gpu.size() == 0) return;

	triangles_TangentV_gpu.clear();
	for (int i=0; i < vertex_positions_triangle_gpu.size(); i+=3)
	{
		// Face's vertex positions
		vec3 vertex1 = vertex_positions_triangle_gpu[i+0];
		vec3 vertex2 = vertex_positions_triangle_gpu[i+1];
		vec3 vertex3 = vertex_positions_triangle_gpu[i+2];
		
		// Edges of triangle
		vec3 e1 = vertex2 - vertex1;
		vec3 e2 = vertex3 - vertex1;

		// Texture coordinates
		vec2 uv1;
		vec2 uv2;
		vec2 uv3;
		if (textureMode == TEXTURE_FROM_FILE)
		{
			uv1 = verticesTextures_original_gpu[i + 0];
			uv2 = verticesTextures_original_gpu[i + 1];
			uv3 = verticesTextures_original_gpu[i + 2];
		}
		else
		{
			uv1 = verticesTextures_canonical_gpu[i + 0];
			uv2 = verticesTextures_canonical_gpu[i + 1];
			uv3 = verticesTextures_canonical_gpu[i + 2];
		}
		
		// Delta of coordinates 
		vec2 delta1 = uv2 - uv1;
		vec2 delta2 = uv3 - uv1;

		float frac = 1.0 / (delta1.x * delta2.y - delta2.x * delta1.y);

		// Calculate Tangent vector:
		vec3 tangent;
		tangent.x = frac*(delta2.y * e1.x - delta1.y * e2.x);
		tangent.y = frac*(delta2.y * e1.y - delta1.y * e2.y);
		tangent.z = frac*(delta2.y * e1.z - delta1.y * e2.z);

		tangent = normalize(tangent);

		// Duplication for VBO size consistency
		triangles_TangentV_gpu.push_back(tangent);
		triangles_TangentV_gpu.push_back(tangent);
		triangles_TangentV_gpu.push_back(tangent);

	}
}

void saveTextureToPNG(const std::vector<float>& data, int width, int height, const char* filename) {
	// Convert texture data to unsigned char
	std::vector<unsigned char> imageData(data.begin(), data.end());
	for (size_t i = 0; i < data.size(); ++i) {
		imageData[i] = static_cast<unsigned char>(data[i] * 255); // Convert float to unsigned char
	}
	// Save texture data to PNG file
	//stbi_write_png(char const* filename, int w, int h, int comp, const void* data, int stride_in_bytes);
	stbi_write_png(filename, width, height, 1, imageData.data(), width * sizeof(unsigned char));
}

void MeshModel::generateMarbleNoise()
{

	marbleTextureBuffer.clear();
	FastNoiseLite noise;
	//noise.SetFrequency(noise_freq);
	
	// Gather noise data
	int width = NOISE_MAP_SIZE;
	int height = NOISE_MAP_SIZE;

	noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);	// Perlin type
	noise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
	noise.SetFrequency(0.03);
	noise.SetFractalOctaves(4);
	noise.SetFractalLacunarity(2);
	noise.SetFractalGain(0.5);

	vector<float> perlinNoise;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float nx = static_cast<float>(x);
			float ny = static_cast<float>(y);
			float value = noise.GetNoise(nx, ny);
			value = (value + 1) / 2.0;	// Normalization to [0,1]
			marbleTextureBuffer.push_back(value);
		}
	}

	//saveTextureToPNG(marbleTextureBuffer, width, height, "marbleTexture.png");

}

void MeshModel::GenerateTexture()
{
	// Texture
	if (textureMap.image_data != nullptr)
	{
		glActiveTexture(GL_TEXTURE0);
		if (tex > 0)
		{
			glBindTexture(GL_TEXTURE_2D, tex);
			glDeleteTextures(1, &tex);
		}

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if(textureMap.channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureMap.width, textureMap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureMap.image_data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureMap.width, textureMap.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureMap.image_data);

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(textureMap.image_data);
	}
}

void MeshModel::GenerateNMap()
{
	// Normal Map
	if (normalMap.image_data != nullptr)
	{
		glActiveTexture(GL_TEXTURE1);
		if (nmap > 0)
		{
			glBindTexture(GL_TEXTURE_2D, nmap);
			glDeleteTextures(1, &nmap);
		}
		glGenTextures(1, &nmap);
		glBindTexture(GL_TEXTURE_2D, nmap);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (textureMap.channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, normalMap.width, normalMap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, normalMap.image_data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, normalMap.width, normalMap.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, normalMap.image_data);
		glGenerateMipmap(GL_TEXTURE_2D); 

		stbi_image_free(normalMap.image_data);
	}
}

void MeshModel::generateMarbleTexture()
{
	generateMarbleNoise();
	
	glActiveTexture(GL_TEXTURE2);
	if (marbletex > 0)
	{
		glBindTexture(GL_TEXTURE_2D, marbletex);
		glDeleteTextures(1, &marbletex);
	}
	glGenTextures(1, &marbletex);
	glBindTexture(GL_TEXTURE_2D, marbletex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, NOISE_MAP_SIZE, NOISE_MAP_SIZE, 0, GL_RED, GL_FLOAT, marbleTextureBuffer.data());

}
