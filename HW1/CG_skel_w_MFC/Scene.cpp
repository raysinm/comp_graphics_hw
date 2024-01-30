#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "CG_skel_w_glfw.h"
#include <string>


using namespace std;

void Scene::AddCamera()
{
	//TODO: Add camera using the input x y z ...


	Camera* cam = new Camera();

	cameras.push_back(cam);
}

void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);

	/*TODO: Ask for user input for name? */

	models.push_back(model);
}

/*  
	Main draw function.
	This will draw:
		1. GUI
		2. scene models
*/
void Scene::draw()
{
	//1. GUI
	drawGUI();
	
	//2. Update the buffer (if needed) before rasterization.
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	m_renderer->update(viewport->WorkSize.x, viewport->WorkSize.y);
	
																	
	//3. TODO: draw all models
	for (auto model : models)
	{
		model->draw(); //rasterization part goes here ??
	}
	m_renderer->SetDemoBuffer(); //debug only. after rasterization the buffer should be already updated.

	
	//4. TODO: draw cameras using '+' signs? 


	//5. Update the texture. (OpenGL stuff)
	m_renderer->updateTexture();
}

void Scene::drawGUI()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Model (.obj file)"))
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
		if (ImGui::BeginMenu("Select"))
		{
			if (ImGui::BeginMenu("Model"))
			{
				int len = models.size();
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

						/* Select / Unselect the model */
						activeModel = models[c]->selected == true ? c : NOT_SELECTED;

						cout << "active model: " << activeModel << endl;
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Camera"))
			{
				int len = cameras.size();
				for (int c = 0; c < len; c++)
				{
					if (ImGui::MenuItem(cameras[c]->name.c_str(), NULL, &cameras[c]->selected))
					{
						/* Deselect all others */
						for (int t = 0; t < len; t++)
						{
							if (t != c)
							{
								cameras[t]->selected = false;
							}
						}

						/* Select current camera */
						cameras[c]->selected = true;
						activeCamera = c;

					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu(); //End select
		}

		ImGui::EndMainMenuBar();
	}
	

	/* Draw the ImGui::Image (Used for displaying our texture) */
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	if (ImGui::Begin("Main Window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | \
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | \
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground))
	{
		// Display the texture in ImGui:
		ImGui::Image((void*)(intptr_t)(m_renderer->m_textureID), viewport->WorkSize);
	}
	ImGui::End();

}