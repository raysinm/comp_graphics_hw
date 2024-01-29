#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

class Model {
protected:
	virtual ~Model() {}
	void virtual draw()=0;

public:
	bool selected = false;
	string name = "Model1";
};


class Light {

};

class Camera {
	mat4 cTransform;
	mat4 projection;

public:
	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	mat4 Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);

	bool selected = false;
	string name = "Camera1";
};

class Scene {

	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	vector<bool> cameras_selected;
	Renderer* m_renderer;

private:
	void AddCamera();

public:
	Scene() {};
	Scene(Renderer *renderer) : m_renderer(renderer) {};
	void loadOBJModel(string fileName);
	void draw();
	void drawDemo();
	void drawGUI();
	friend void imguiDemoRun();

	int activeModel;
	int activeLight;
	int activeCamera;
};