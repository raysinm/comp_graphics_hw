#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
#include "Grid.h"

using namespace std;


#define NOT_SELECTED -1
#define ADD_INPUT_POPUP_TITLE "Object Properties"
#define CAMERA_DEFAULT_NAME "Camera"
#define MODEL_DEFAULT_NAME "Model"

#define DEF_PARAM_RANGE 20;
#define DEF_PARAM 10;
#define DEF_ZNEAR 1;
#define DEF_ZFAR 100;
#define DEF_FOV 45
#define DEF_ASPECT 1


class Model
{
protected:
	virtual ~Model() {}
	string name = MODEL_DEFAULT_NAME;
	bool userInitFinished = false;
public:
	void virtual draw(mat4& cTransform, mat4& projection) = 0;

	void setName(std::string newName) { name = newName; }
	void SetUserInitFinished() { userInitFinished = true; }
	bool GetUserInitFinished() { return userInitFinished; }

	std::string& getName() { return name; }
	bool selected = false;

};


class Light {

};

class Camera
{
private:
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	string name = "";
	float c_left, c_right, c_top, c_bottom, c_fovy , c_aspect , c_zNear, c_zFar;
	vec4 c_trnsl, c_rot;

	friend class Scene;	// To acces transformations;

public:

	mat4 cTransform;
	mat4 view_matrix;	// cTransform inversed
	mat4 projection;

	Camera::Camera();
	void setTransformation(const mat4& transform);	//TODO: change to vectors for trnsl, rot
	void LookAt(const Model* target);		//Called from keyboard event.
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
	void setFovAspectByParams();
	void setParamsByFovAspect();
	void resetProjection();
	
	
	void setName(std::string newName) { name = newName; }
	std::string& getName() { return name; }
	
	void updateTransform();
	void ResetTranslation() { c_trnsl = vec4(0); }
	void ResetRotation() { c_rot = vec4(0); }
	
	bool selected = false;
};

class Scene {

	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer* m_renderer;
	Grid* grid;
	vec2* bufferGrid;

private:
	void AddCamera();
	void UpdateModelSelection();

	void ResetPopUpFlags();
	bool GUI_popup_pressedOK = false, GUI_popup_pressedCANCEL = false;
	bool showGrid = false;
	int viewportX;
	int viewportY;
	int viewportWidth;
	int viewportHeight;

public:
	Scene(Renderer* renderer) : m_renderer(renderer)
	{
		AddCamera();							//Add the first default camera
		activeCamera = 0;						//index = 0 because it is the first
		cameras[activeCamera]->selected = true; //Select it because it is the default
		grid = new Grid();
	};
	void loadOBJModel(string fileName);
	void draw();
	void drawGUI();
	void resize_callback_handle(int width, int height);
	void setViewPort(vec4& vp);
	friend bool showInputDialog();

	Camera* GetActiveCamera();
	Model* GetActiveModel();

	int activeModel  = NOT_SELECTED;
	int activeLight  = NOT_SELECTED;
	int activeCamera = 0;	// Always at least one camera
};