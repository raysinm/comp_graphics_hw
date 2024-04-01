#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_map>
#include "CG_skel_w_glfw.h"
#include "Poly.h"
#include "Utils.h"
#include "GL/glew.h"
#include "Light.h"


using namespace std;
class MeshModel;
class Model;

class Renderer
{
private:
	GLFWwindow* m_window;
	int m_width, m_height;
	int DEFAULT_BACKGROUND_COLOR = 0;
	vec3 DEFAULT_WIREFRAME_COLOR = vec3(1.0, 1.0, 1.0);

	//unordered_map<vec2, vec3, vec2Hash> highlightPixels;
	//float* m_outBuffer_screen;			// 3*width*height
	//float* m_outBuffer_antialiasing;
	//float* m_outBuffer;
	//float* m_outBuffer_fsblur;	// 3*width*height
	//UINT* m_zbuffer;   // width*DEF_SUPERSAMPLE_SCALE*height*DEF_SUPERSAMPLE_SCALE	
	//int true_width, true_height;
	//int m_max_obj_y, m_min_obj_y;
	//vector<vector<float>> kernel;
	//vector<vector<float>> kernelFSBLUR;
	//void CreateBuffers();
	//void DrawLine(vec2 A, vec2 B, bool isNegative, vec4 color);
	//void ComputePixels_Bresenhams(vec2 A, vec2 B, bool flipXY, int y_mul, vec4 color);
	//void ScanLineZ_Buffer(vector<Poly>& polygons);
	//void PutColor(UINT x, UINT y, vec3& color);
	//vec3 GetColor(vec3& pixl, Poly& p);
	//vector<Poly> CreatePolygonsVector(const MeshModel* model);
	//void UpdateMinMaxY(Poly& P);
	//std::pair<int, int> CalcScanlineSpan(Poly& p, int y);
	//void calcIntensity(Light* lightSource, vec3& Ia_total, vec3& Id_total, vec3& Is_total, vec3& P, vec3& N, Material& mate);
	//vector<vector<float>> generateGaussianKernel(int size, float sigma);
	//float prevFSblurSigma;
	//float prevbloomFilterSigma;

	
	//////////////////////////////
	// openGL stuff. touch.

	//void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
	
public:
	GLuint program	  = -1;
	GLuint UBO_lights = -1;
	
	Renderer(int width, int height, GLFWwindow* window);
	~Renderer(void);
	void SwapBuffers();


	//void Rasterize_WireFrame(const Vertex* vertices, unsigned int len, vec4 color);
	//void Rasterize_WireFrame(const Vertex* vertices, unsigned int len);
	//void Rasterize(const MeshModel* model);
	//void SetBufferLines(const vec2* points, unsigned int len, vec4 color);
	//void SetBufferLines(const vec2* points, unsigned int len);
	//void ApplyBloomFilter();
	//void ApplyFullScreenBlur();
	//void gaussianBlur(const float* image, float* blurredImage, vector<vector<float>>& kernelToUse);
	//void sampleAntialias();
	//void update(int width, int height);
	//void updateBuffer();
	//void ResetMinMaxY();
	
	void CreateTexture();
	vec2 GetBufferSize();
	vec2 GetWindowSize();
	void updateTexture();
	void clearBuffer();
	void invertSceneColors()
	{
		DEFAULT_WIREFRAME_COLOR = vec3(1) - DEFAULT_WIREFRAME_COLOR;
		DEFAULT_BACKGROUND_COLOR = 1 - DEFAULT_BACKGROUND_COLOR;
	}
	void drawModel(DrawAlgo draw_algo, Model* model, mat4& cTransform);
	void UpdateLightsUBO(bool reallocate_ubo);
	void calcResolution(int* w, int* h) { glfwGetFramebufferSize(m_window, w, h); }

};
