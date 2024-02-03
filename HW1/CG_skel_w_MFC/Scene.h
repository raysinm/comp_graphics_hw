#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

#define NOT_SELECTED -1
#define ADD_INPUT_POPUP_TITLE "Object Properties"
#define CAMERA_DEFAULT_NAME "Camera"

class Model {
protected:
	virtual ~Model() {}
	string name = "Model";

public:
	void virtual draw(mat4& cTransform, mat4& projection)=0;
	void setName(std::string newName) { name = newName; }
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
	float m_left, m_right, m_top, m_bottom, m_fovy, m_aspect, m_zNear, m_zFar;
	vec4 m_trnsl, m_rot;

	friend class Scene;	// To acces transformations;

public:

	mat4 cTransform;
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
	void setName(std::string newName) { name = newName; }
	std::string& getName() { return name; }
	bool selected = false;
};

class Scene {

	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer* m_renderer;

private:
	void AddCamera();

	void ResetPopUpFlags();
	bool GUI_popup_pressedOK = false, GUI_popup_pressedCANCEL = false;

public:
	Scene(Renderer* renderer) : m_renderer(renderer)
	{
		AddCamera();							//Add the first default camera
		activeCamera = 0;						//index = 0 because it is the first
		cameras[activeCamera]->selected = true; //Select it because it is the default
	};
	void loadOBJModel(string fileName);
	void draw();
	void drawGUI();
	friend bool showInputDialog();
	Camera* GetActiveCamera();
	Model* GetActiveModel();

	int activeModel  = NOT_SELECTED;
	int activeLight  = NOT_SELECTED;
	int activeCamera = NOT_SELECTED;
};