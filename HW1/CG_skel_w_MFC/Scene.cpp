#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "CG_skel_w_glfw.h"
#include <string>


using namespace std;

void Scene::AddCamera()
{
	//TODO: Add camera using the input x y z ...
}

void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);

	/*TODO: Ask for user input for name? */

	models.push_back(model);
	cameras_selected.push_back(false);
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
}

void Scene::drawGUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Add..."))
		{
			if (ImGui::MenuItem("Model (.obj)"))
			{
				CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("(*.obj)|*.obj|All Files (*.*)|*.*||"));
				if (dlg.DoModal() == IDOK)
				{
					std::string s((LPCTSTR)dlg.GetPathName());
					loadOBJModel(s);
				}
			}
			if (ImGui::MenuItem("Camera"))
			{
				AddCamera();
			}
			
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Models"))
		{
			int len = models.capacity();
			for (int c = 0; c < len; c++)
			{
				if (ImGui::MenuItem(models[c]->name.c_str(), NULL, &models[c]->selected))
				{
					/* Deselect all others */
					for (int t = 0; t < len; t++)
					{
						if (t != c)
						{
							models[t]->selected = false;
						}
					}

					
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}