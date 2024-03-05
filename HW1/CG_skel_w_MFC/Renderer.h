#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <algorithm>
#include "CG_skel_w_glfw.h"
//#include "vec.h"
//#include "mat.h"
#include "Poly.h"
#include "Utils.h"
#include "GL/glew.h"



using namespace std;


class MeshModel;

class Renderer
{
private:

	float* m_outBuffer; // 3*width*height
	UINT* m_zbuffer;   // width*height
	int m_width, m_height;
	int m_max_obj_y, m_min_obj_y;
	GLFWwindow* m_window;	// For glfw swap buffers

	void CreateBuffers(int width, int height);
	void DrawLine(vec2 A, vec2 B, bool isNegative, vec4 color = vec4(0, 0, 0, 1));
	void ComputePixels_Bresenhams(vec2 A, vec2 B, bool flipXY, int y_mul, vec4 color= vec4(0, 0, 0, 1));
	void ScanLineZ_Buffer(vector<Poly>& polygons);
	void PutColor(UINT x, UINT y, vec4& color);

	vector<Poly> CreatePolygonsVector(const MeshModel* model);
	void UpdateMinMaxY(Poly& P);


	std::pair<int, int> CalcScanlineSpan(Poly& p, int y);

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
	

public:
	Renderer(int width, int height, GLFWwindow* window);
	~Renderer(void);
	void SwapBuffers();


	void Rasterize_WireFrame(const Vertex* vertices, unsigned int len, vec4 color = vec4(0, 0, 0, 1));
	void Rasterize_Flat(const MeshModel* model);
	void Rasterize_Gouraud(const MeshModel* model);
	void Rasterize_Phong(const MeshModel* model);

	void SetBufferLines(const vec2* points, unsigned int len, vec4 color = vec4(0, 0, 0, 1));

	// New funcs
	void CreateTexture();
	vec2 GetBufferSize();
	vec2 GetWindowSize();
	void update(int width, int height);
	void updateTexture();
	void updateBuffer();
	void clearBuffer();
	void ResetMinMaxY();

	GLuint m_textureID;

};
