#pragma once

#include "mat.h"

#define RED   0
#define GREEN 1
#define BLUE  2
#define MAX_Z 65535
#define LIGHT_DEFAULT_NAME "Light"
#define EPSILON 0.00001
#define DEFUALT_EMIS_FACTOR 0.15
#define DEFUALT_LIGHT_Ka_VALUE 0.5
#define DEFUALT_LIGHT_Kd_VALUE 1
#define DEFUALT_LIGHT_Ks_VALUE 0
#define DEFUALT_LIGHT_K_VALUE 0.5
#define DEFUALT_LIGHT_LA_VALUE 0.5
#define DEFUALT_LIGHT_LD_VALUE 1
#define DEFUALT_LIGHT_LS_VALUE 0
#define DEFUALT_LIGHT_ALPHA 1
#define DEF_SUPERSAMPLE_SCALE 2
#define NOT_SELECTED -1
#define ADD_INPUT_POPUP_TITLE "Object Properties"
#define CAMERA_DEFAULT_NAME "Camera"
#define MODEL_DEFAULT_NAME "Model"
#define DEF_PARAM_RANGE 20;
#define DEF_PARAM 1;
#define DEF_ZNEAR 1;
#define DEF_ZFAR 20;
#define DEF_FOV 45
#define DEF_ASPECT 1
#define DEF_MAX_FOG_EFFECT 100
#define NOISE_MAP_SIZE 512

#define DEF_VEIN_FREQ 20
#define DEF_VEIN_THICKNESS 3
#define DEF_COL_MIX_FACTOR 0.8
#define DEF_NOISE_FREQ 1.0
#define DEF_NOISE_OCTAVES 3
#define DEF_NOISE_AMPLITUDE 8.0

//#define DEF_NOISE_SCALE 100
//#define DEF_NOISE_LACUNARITY 5.54


typedef unsigned int UINT;

inline int Index(int width, int x, int y, int c) { return (x + y * width) * 3 + c; }
inline int Z_Index(int width, int x, int y) { return x + (y * width); }


enum MODEL_OBJECT {
	MODEL_WIREFRAME,
	MODEL_TRIANGLES,
	BBOX,
	V_NORMAL,
	F_NORMAL
};


enum VAO_INDICES {
	VAO_VERTEX_WIREFRAME = 0,
	VAO_VERTEX_TRIANGLE	 = 1,
	VAO_VERTEX_BBOX 	 = 2,
	VAO_VERTEX_VNORMAL 	 = 3,
	VAO_VERTEX_FNORMAL 	 = 4,
	VAO_COUNT,
};

enum VBO_INDICES {
	VBO_VERTEX_POS			= 0,
	VBO_VERTEX_FACE_POS,
	VBO_VERTEX_VN,
	VBO_VERTEX_FN,
	VBO_VERTEX_DIFFUSE_COLOR_FLAT,
	VBO_VERTEX_DIFFUSE_COLOR,
	VBO_VERTEX_TEXTURE_MAP,
	VBO_FACE_TANGENT,
	VBO_COUNT,
};


enum DrawAlgo {
	WIRE_FRAME = 0,
	FLAT,
	GOURAUD,
	PHONG,
	COUNT
};

typedef struct Vertex {
	vec3 point;
	vec3 point_cameraspace;
	UINT vertex_index;
	UINT face_index;

	Vertex() : point(0, 0, 0), vertex_index(0), face_index(0) {}
	Vertex(vec3& a, UINT v_indx, UINT f_indx, vec3& a_cameraspace) { point = a; vertex_index = v_indx; face_index = f_indx; point_cameraspace = a_cameraspace; }
	~Vertex() {}
} Vertex;

class Line {
private:
	double _slope;
	double _b;

	bool isVertical=false;
	double verticalX =0;


	vec2 a, b;
public:
	Line() : _slope(1), _b(0){}
	Line(double slope, double b) : _slope(slope), _b(b){}
	Line(vec2& a, vec2& b)
	{
		this->a = a;
		this->b = b;

		isVertical = (abs(a.x - b.x) < EPSILON);

		if (!isVertical) {
			_slope = (b.y - a.y) / (b.x - a.x);
			_b = (a.y - _slope * a.x);
		}
		else
		{
			verticalX = a.x;
		}
	}
	bool operator==(Line& other)
	{
		return ( abs(this->_slope - other._slope) < EPSILON &&\
				 abs(this->_b - other._b) < EPSILON);
	}

	vec2 intersect(Line& other, bool* isParallel)
	{
		/* We assume that *this line is a scanline (slope = 0) */
		/* Handle Vertical line */
		if (other.getIsVertical())
		{
			if (this->isVertical)
			{
				*isParallel = true;
				if (abs(this->verticalX - other.verticalX) < EPSILON)
					return vec2(verticalX, 0);
				else
					return vec2(0, 0);
			}
			else
				return vec2(other.getVerticalX(), this->y(other.getVerticalX()));
		}
		if (this->isVertical)
		{
			if (other.getIsVertical())
			{
				*isParallel = true;
				if (abs(this->verticalX - other.verticalX) < EPSILON)
					return vec2(verticalX, 0);
				else
					return vec2(0, 0);
			}
			else
				return vec2(this->verticalX, other.y(this->verticalX));
		}


		/* Handle Parallel lines */
		if (abs(this->_slope - other._slope) < EPSILON)
		{
			//throw ParallelLinesException("Parallel Lines");
			*isParallel = true;
			return vec2(0);
		}
		else
			*isParallel = false;
		/* Handle all other lines */
		double x = (other._b - this->_b) / (this->_slope - other._slope);
		double y = this->y(x);

		return vec2(x, y);
	}
	double y(double x) { 
		return (_slope * x + _b);
	}

	bool getIsVertical() { return isVertical; }
	double getVerticalX() { return verticalX; }
	double getSlope() { return _slope; }
	vec2& getA() { return a; }
	vec2& getB() { return b; }
	bool isParallel(Line& other)
	{
		if (isVertical)
			return other.getIsVertical();

		return abs(_slope - other.getSlope()) < EPSILON;
	}
};


typedef enum LIGHT_TYPE
{
	AMBIENT_LIGHT	= 0,
	POINT_LIGHT		= 1,
	PARALLEL_LIGHT	= 2
}LIGHT_TYPE;


struct LightProperties
{
	vec4 position;
	vec4 dir;
	vec4 color;
	float La;
	float Ld;
	float Ls;
	int type;  // Ambient=0 / Point=1 / parallel=2
};

typedef struct _STB_Image
{
	unsigned char* image_data;
	//float* image_data_normalized;
	int width, height, channels;

	//void normalizeImage()
	//{
	//	if (!image_data) return;

	//	image_data_normalized = new float[width * height * channels];
	//	if (!image_data_normalized) return;

	//	for (int index = 0; index < width * height * channels; index++)
	//		image_data_normalized[index] = image_data[index] / 255.0f;
	//}

} STB_Image;

typedef enum ColorAnimationType
{
	COLOR_ANIMATION_STATIC = 0,
	COLOR_ANIMATION_1,
	COLOR_ANIMATION_2,
} ColorAnimationType;

typedef enum TextureMode
{
	TEXTURE_FROM_FILE = 0,
	TEXTURE_CANONICAL_1,		// Project on XY place
	TEXTURE_CANONICAL_2,		// Spherical 
} TextureMode;