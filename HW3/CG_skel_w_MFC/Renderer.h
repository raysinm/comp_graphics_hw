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

class Renderer
{
private:
	unordered_map<vec2, vec3, vec2Hash> highlightPixels;
	float* m_outBuffer_screen;			// 3*width*height
	float* m_outBuffer_antialiasing;
	float* m_outBuffer;
	float* m_outBuffer_fsblur;	// 3*width*height
	UINT* m_zbuffer;   // width*DEF_SUPERSAMPLE_SCALE*height*DEF_SUPERSAMPLE_SCALE	
	int m_width, m_height, true_width, true_height;
	int m_max_obj_y, m_min_obj_y;
	GLFWwindow* m_window;	// For glfw swap buffers
	vector<vector<float>> kernel;
	vector<vector<float>> kernelFSBLUR;


	vec4 DEFAULT_WIREFRAME_COLOR = vec4(1.0, 1.0, 1.0);
	int DEFAULT_BACKGROUND_COLOR = 0;

	void CreateBuffers();
	void DrawLine(vec2 A, vec2 B, bool isNegative, vec4 color);
	void ComputePixels_Bresenhams(vec2 A, vec2 B, bool flipXY, int y_mul, vec4 color);
	void ScanLineZ_Buffer(vector<Poly>& polygons);
	void PutColor(UINT x, UINT y, vec3& color);
	vec3 GetColor(vec3& pixl, Poly& p);
	vector<Poly> CreatePolygonsVector(const MeshModel* model);
	void UpdateMinMaxY(Poly& P);
	std::pair<int, int> CalcScanlineSpan(Poly& p, int y);
	void calcIntensity(Light* lightSource, vec3& Ia_total, vec3& Id_total, vec3& Is_total, vec3& P, vec3& N, Material& mate);
	vector<vector<float>> generateGaussianKernel(int size, float sigma);
	float prevFSblurSigma;
	float prevbloomFilterSigma;


	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
	
public:
	GLuint VAO_vertex_pos;
	GLuint program;
	GLuint m_textureID;
	float bloom_filter_threshold = 1.0f;
	float bloom_filter_factor = 0.5f;
	int fs_blur_iterations = 1;
	bool ss_antialias = false;

	int kernelFSBlurSize = 11;
	float FSblurSigma = 2;

	int kernelbloomFilterSize = 11;
	float kernelbloomFilterSigma = 2;


	Renderer(int width, int height, GLFWwindow* window);
	~Renderer(void);
	void SwapBuffers();


	void Rasterize_WireFrame(const Vertex* vertices, unsigned int len, vec4 color);
	void Rasterize_WireFrame(const Vertex* vertices, unsigned int len);
	void Rasterize(const MeshModel* model);


	void SetBufferLines(const vec2* points, unsigned int len, vec4 color);
	void SetBufferLines(const vec2* points, unsigned int len);
	void ApplyBloomFilter();
	void ApplyFullScreenBlur();
	void gaussianBlur(const float* image, float* blurredImage, vector<vector<float>>& kernelToUse);
	void sampleAntialias();

	// New funcs
	void CreateTexture();
	vec2 GetBufferSize();
	vec2 GetWindowSize();
	void update(int width, int height);
	void updateTexture();
	void updateBuffer();
	void clearBuffer();
	void ResetMinMaxY();

	void invertSceneColors() {
		DEFAULT_WIREFRAME_COLOR = vec4(1) - DEFAULT_WIREFRAME_COLOR;
		DEFAULT_BACKGROUND_COLOR = 1 - DEFAULT_BACKGROUND_COLOR;
	}

};
