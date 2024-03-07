#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
#include "Grid.h"
#include "Light.h"
#include "Utils.h"

using namespace std;


#define NOT_SELECTED -1
#define ADD_INPUT_POPUP_TITLE "Object Properties"
#define CAMERA_DEFAULT_NAME "Camera"
#define MODEL_DEFAULT_NAME "Model"

#define DEF_PARAM_RANGE 20;
#define DEF_PARAM 10;
#define DEF_ZNEAR (1);
#define DEF_ZFAR (20);
#define DEF_FOV 45
#define DEF_ASPECT 1



class Model
{
protected:
	virtual ~Model() {}
	string name = MODEL_DEFAULT_NAME;
	bool userInitFinished = false;
public:
	void virtual draw(mat4& cTransform, mat4& projection, bool allowClipping, mat4& cameraRot) = 0;


	void setName(std::string newName) { name = newName; }
	void SetUserInitFinished() { userInitFinished = true; }
	bool GetUserInitFinished() { return userInitFinished; }

	std::string& getName() { return name; }
	bool selected = false;

};



class Camera
{
private:
	mat4 LookAt(const vec4& eye, const vec4& at, const vec4& up);

	string name = "";
	float c_left, c_right, c_top, c_bottom, c_fovy , c_aspect , c_zNear, c_zFar;
	vec4 c_trnsl, c_rot, c_trnsl_viewspace, c_rot_viewspace;
	vec4 target;

	mat4 transform_mid_worldspace, transform_mid_viewspace;
	
	// Icon stuff
	vec3* icon;
	vec2* iconBuffer;

	unsigned int num_icon_vertices;

	bool lockFov_GUI = false;

	friend class Scene;	// To acces transformations;

public:

	mat4 cTransform;
	mat4 view_matrix;	// cTransform inversed
	mat4 projection;
	mat4 rotationMat_normals;

	Camera::Camera();
	
	void Camera::LookAt(const Model* target = nullptr);

	void Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );	// Sets projection matrix
	mat4 Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);	// Calls frustum

	void setOrtho();
	void setPerspective();
	void setPerspectiveByFov();
	void setPerspectiveByParams();

	void resetProjection();
	void zoom(double s_offset, double update_rate = 0.1);
	
	void setName(std::string newName) { name = newName; }
	std::string& getName() { return name; }
	vec4 getTranslation() { return vec4(c_trnsl); }
	void setStartPosition(vec4& pos) { c_trnsl = pos; }
	
	void updateTransform();
	void ResetTranslation() { c_trnsl = vec4(0,0,10,1); }
	void ResetRotation() { c_rot = vec4(0,0,0,1); }
	
	void iconInit();
	bool iconDraw(mat4& active_cTransform, mat4& active_projection);
	vec2* getIconBuffer() { return iconBuffer; }
	unsigned int getIconBufferSize() { return num_icon_vertices; }


	void ResetTranslation_viewspace() { c_trnsl_viewspace = vec4(0, 0, 0, 1); }
	void ResetRotation_viewspace() { c_rot_viewspace = vec4(0,0,0,1); }
	
	void unLockFovy() { lockFov_GUI = false; }
	bool* getLockFovyPTR() { return &lockFov_GUI; }
	bool selected = false;
	bool isOrtho = true;
	bool renderCamera = false;
	bool allowClipping = true;
};

class Scene {

private:
	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer* m_renderer;
	DrawAlgo draw_algo = WIRE_FRAME;

	void AddCamera();
	void AddLight();
	void UpdateModelSelection();

	void ResetPopUpFlags();
	void drawCameraTab();
	void drawModelTab();
	void drawLightTab();
	bool GUI_popup_pressedOK = false, GUI_popup_pressedCANCEL = false;
	bool showGrid = false;
	int viewportX;
	int viewportY;
	int viewportWidth;
	int viewportHeight;
	char* drawAlgoToString(DrawAlgo x)
	{
		switch (x)
		{
		case WIRE_FRAME:
			return "WIRE FRAME";
		case FLAT:
			return "FLAT";
		case GOURAUD:
			return "GOURAUD";
		case PHONG:
			return "PHONG";
		default:
			return ("error");
		}
	}

public:
	Scene(Renderer* renderer) : m_renderer(renderer)
	{
		AddCamera();							 //Add the first default camera
		AddLight ();							 //Add the first default ambient light
		
		activeCamera = 0;						 //index = 0 because it is the first
		activeLight  = 0;						 //index = 0 because it is the first
		cameras[activeCamera]->selected = true;  //Select it because it is the default
		lights[activeLight]->selected   = true;  //Select it because it is the default
	};
	void loadOBJModel(string fileName);
	void draw();
	void drawGUI();
	void resize_callback_handle(int width, int height);
	void setViewPort(vec4& vp);
	void zoom(double s_offset) { cameras[activeCamera]->zoom(s_offset); }
	friend bool showInputDialog();

	Camera* GetActiveCamera();
	Model* GetActiveModel();
	Light* GetActiveLight();

	int activeModel  = NOT_SELECTED;
	int activeLight  = NOT_SELECTED;
	int activeCamera = 0;	// Always at least one camera
};