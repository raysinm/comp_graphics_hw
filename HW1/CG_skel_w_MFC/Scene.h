#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

#define NOT_SELECTED -1

class Model {
protected:
	virtual ~Model() {}

public:
	void virtual draw(mat4& cTransform)=0;
	void setName(std::string newName) { name = newName; }
	bool selected = false;
	string name = "Model";

};


class Light {

};

class Camera
{

public:
	mat4 cTransform;
	mat4 projection;

	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );	// Calculates frustum?
	mat4 Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);
	void setName(std::string newName) { name = newName; }
	bool selected = false;
	string name = "Camera1";
};

class Scene {

	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer* m_renderer;

private:
	void AddCamera();

public:
	Scene() {};
	Scene(Renderer*   renderer) : m_renderer(renderer) {};
	void loadOBJModel(string fileName);
	void draw();
	void drawGUI();
	friend bool showInputDialog();
	//friend void showInputDialog();
	int activeModel;
	int activeLight;
	int activeCamera;
};