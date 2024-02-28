#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include "CG_skel_w_glfw.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"


typedef unsigned int UINT;

typedef struct Vertex {
	vec3 point;
	UINT index;

	Vertex() : point(0, 0, 0), index(0) {}
	Vertex(vec3 a, UINT indx) { point = a; index = indx; }
	~Vertex() {}
} Vertex;


using namespace std;

class MeshModel;

class Renderer
{
private:

	float* m_outBuffer; // 3*width*height
	UINT* m_zbuffer;   // width*height
	int m_width, m_height;
	GLFWwindow* m_window;	// For glfw swap buffers

	void CreateBuffers(int width, int height);

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
	
	void DrawLine(vec2 A, vec2 B, bool isNegative, vec4 color = vec4(0, 0, 0, 1));
	void ComputePixels_Bresenhams(vec2 A, vec2 B, bool flipXY, int y_mul, vec4 color= vec4(0, 0, 0, 1));

public:
	Renderer(int width, int height, GLFWwindow* window);
	~Renderer(void);
	void SwapBuffers();


	void Rasterize_WireFrame(const Vertex* vertecies, unsigned int len, vec4 color = vec4(0, 0, 0, 1));
	void Rasterize_Flat(const MeshModel* model);
	void SetBufferLines(const vec2* points, unsigned int len, vec4 color = vec4(0, 0, 0, 1));

	// New funcs
	void CreateTexture();
	vec2 GetBufferSize();
	vec2 GetWindowSize();
	void update(int width, int height);
	void updateTexture();
	void updateBuffer();
	void clearBuffer();

	GLuint m_textureID;

};
