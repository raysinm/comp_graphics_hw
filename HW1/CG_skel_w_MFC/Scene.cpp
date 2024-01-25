#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "CG_skel_w_MFC.h"
#include <string>

using namespace std;
void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	
	
	m_renderer->DrawBtns();
	m_renderer->SwapBuffers();
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}
