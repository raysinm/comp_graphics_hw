#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include "CG_skel_w_glfw.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"


using namespace std;
class Renderer
{
	float *m_zbuffer; // width*height
	int m_width, m_height;
	GLFWwindow* m_window;	// For glfw swap buffers

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
	
public:
	Renderer(int width, int height,GLFWwindow* window);
	~Renderer(void);
	void Init();
	void DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* normals=NULL);
	void SetCameraTransform(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void SetBufferOfModel(vec2* vertecies, unsigned int len, vec4 color=vec3(0));
	void SetBufferLines(vec2* points, unsigned int len, vec4 color = vec3(0));

	// New funcs
	void CreateTexture();
	vec2 GetBufferSize();
	void update(int width, int height);
	void updateTexture();
	void updateBuffer();
	void clearBuffer();

	GLuint m_textureID;
private:

	void DrawLine(vec2 A, vec2 B, bool isNegative, vec4 color = vec3(0));
	void ComputePixels_Bresenhams(vec2 A, vec2 B, bool flipXY, int y_mul, vec4 color=vec3(0));
	float* m_outBuffer; // 3*width*height

};
