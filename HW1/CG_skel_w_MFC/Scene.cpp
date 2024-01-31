#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "CG_skel_w_glfw.h"
#include <string>


using namespace std;
static char modelName[15];

void Scene::AddCamera()
{
	//TODO: Add camera using the input x y z ...


	Camera* cam = new Camera();

	cameras.push_back(cam);
}

void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);
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
#ifdef _DEBUG
	m_renderer->SetDemoBuffer(); //debug only. after rasterization the buffer should be already updated.
#endif

	
	//4. TODO: draw cameras using '+' signs? 


	//5. Update the texture. (OpenGL stuff)
	m_renderer->updateTexture();
}

void Scene::drawGUI()
{
	bool showObjDialog = false;
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
					std::string filename((LPCTSTR)dlg.GetPathName());
					loadOBJModel(filename);
					showObjDialog = true;
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

		//-----------------------------------
		//--------------- Pop up model input dialog
		Model* currModel;
		//char modelName[15] = currModel->name.c_str();	//TODO
		/*char modelName[15] = "Model";*/
		char tempModelBuffer[15];
		float position[3] = { 0,0,0 };	// Let user choose where to put it - by default 0,0,0. Translate ;
		bool pressedOK = false;

		// Check if the popup should be shown
		if (showObjDialog) {
			currModel = models.back();
			strncpy(modelName, currModel->name.c_str(), IM_ARRAYSIZE(modelName));
			ImGui::OpenPopup("MyPopup");
			showObjDialog = false; // Reset the flag
		}

		// Define the contents of the modal popup
		if (ImGui::BeginPopupModal("MyPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			// Add input controls for the name, x, y, and z fields
			//strncpy(tempModelBuffer, modelName, IM_ARRAYSIZE(tempModelBuffer));

			ImGui::InputText("Name", modelName, IM_ARRAYSIZE(modelName));
			
			// Validate name input
			//bool isNameValid = (tempModelBuffer[0] != '\0');

			//// If you want to store the current value for the next frame, you can update the buffer
			//if (isNameValid) {
			//	// Update the name with the current buffer value
			//	//strncpy(modelName, tempModelBuffer, IM_ARRAYSIZE(modelName));
			//	
			//}

			ImGui::InputFloat3("Position", position);
			
			// Validate position input
			bool arePositionValuesValid = true;
			for (int i = 0; i < 3; ++i) {
				arePositionValuesValid = arePositionValuesValid &&
					!std::isnan(position[i]) &&
					!std::isinf(position[i]);
			}		//&&
					//isValueInRange(position[i], minPositionValue, maxPositionValue);
			

			// Notify the user if position values are not in the specified range
			if (!arePositionValuesValid) {
				//ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Position values must be in the range [%f, %f].", minPositionValue, maxPositionValue);
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Position values must be float");
			}

			// Add buttons for OK and Cancel
			if (ImGui::Button("OK") && arePositionValuesValid) {
				// Set flag to indicate OK button is pressed
				pressedOK = true;
				ImGui::CloseCurrentPopup(); // Close the popup
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel")) {
				// Close the popup without adding the model
				models.pop_back();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		// Check if OK button was pressed in the popup
		if (pressedOK) {
			// Perform actions based on user input (e.g., save values, etc.)
			// ...
			auto currModel = models.back();	// Surely loaded new model
			currModel->setName(modelName);
			//TODO: translate the model to given position;

			// Reset the flag
			pressedOK = false;
		}
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