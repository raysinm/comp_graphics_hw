#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "CG_skel_w_glfw.h"
#include <string>
#include <math.h>

using namespace std;
static char nameBuffer[15];	// Supposed to be static?
static float posBuffer[3] = { 0,0,0 };
static bool modelOK = false, camOK = false, showModelDialog = false, showCamDialog = false;


//-----------------
//---------- CAMERA

Camera::Camera()
{
	// Default camera projection - Perspective
	Frustum(-1, 1, -1, 1, 1, 15);
	name = "Default Camera";
}
// TODO: Test
void Camera::Ortho(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{
	if (zNear >= zFar)
	{
		// Illegal	//TODO
	}
	// Sets orthographic projection
	GLfloat x = right - left;
	GLfloat y = top - bottom;
	GLfloat z = zFar - zNear;
	projection = mat4(2 / x	, 0		, 0		, -(left + right) / x,
					  0		, 2 / y	, 0		, -(top + bottom) / y,
					  0		, 0		, -2 / z, -(zFar + zNear) / z,
					  0		, 0		, 0		, 1	);
}

// TODO: Test
void Camera::Frustum(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar) {
	// Sets user-requested frostum	
	if (zNear >= zFar)
	{
		// Illegal	//TODO
	}

	GLfloat x = right - left;
	GLfloat y = top - bottom;
	GLfloat z = zFar - zNear;

	projection = mat4(2*zNear / x	, 0				, (right+left)/x	, 0,
					  0				, 2*zNear / y	, (top + bottom) / y, 0,
					  0				, 0				, -(zFar+zNear) / z	, -(2*zNear*zFar)/z,
					  0				, 0				, -1				, 0);

}

// TODO: Test
mat4 Camera::Perspective(const float fovy, const float aspect,
	const float zNear, const float zFar)
{
	if (fovy > 90)
	{
		// Not a frustum	//TODO
	}
	if (zNear >= zFar)
	{
		// Illegal	//TODO
	}
	float top, bottom, right, left;
	top = zNear * tanf(M_PI/180*(fovy) / 2);
	bottom = -top;
	right = top * aspect;
	left = -right;

	Frustum(left, right, bottom, top, zNear, zFar);
	return projection;
}

//-----------------
//----------- SCENE

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
	
																	
	//3. draw each MeshModel
	for (auto model : models)
	{
		model->draw(cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);

		//3.5 Projection
		// Uses camera 
		// projection =Ortho * vertex
		// normalized_projection = ST * projection



		// 4. Resterization

		//This should hold the MeshModel vertices AFTER all Transforms and projection.
		//values: [-1, 1]
		vec2* vertecies = ((MeshModel*)model)->Get2dBuffer();
		m_renderer->SetBufferOfModel(vertecies); //Rasterazation process...
	}
#ifdef _DEBUG
	//m_renderer->SetDemoBuffer(); //debug only. after rasterization the buffer should be already updated.
#endif

	//4. TODO: draw cameras using '+' signs? 


	//5. Update the texture. (OpenGL stuff)
	m_renderer->updateTexture();
}


void Scene::drawGUI()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(0, 0);
	style.ItemSpacing   = ImVec2(5, 4);
	style.FramePadding  = ImVec2(4, 10);

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
					showModelDialog = true;
					//ImGui::OpenPopup("ModelPopup");
					
					auto currModel = models.back();
					strncpy(nameBuffer, currModel->name.c_str(), IM_ARRAYSIZE(nameBuffer));

				}
			}
			if (ImGui::MenuItem("Camera"))
			{
				AddCamera();
				showCamDialog = true;

				auto currCamera = cameras.back();
				strncpy(nameBuffer, currCamera->name.c_str(), IM_ARRAYSIZE(nameBuffer));

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


	//Check if the popup should be shown
	if (showModelDialog) {
	//	currModel = models.back();
	//	strncpy(nameBuffer, currModel->name.c_str(), IM_ARRAYSIZE(nameBuffer));
		ImGui::OpenPopup("inputPopUp");
	}
	if (showCamDialog) {
	//	currCamera = cameras.back();
	//	strncpy(nameBuffer, currCamera->name.c_str(), IM_ARRAYSIZE(nameBuffer));
		ImGui::OpenPopup("inputPopUp");
	//	
	}

	

	//bool pressedOK = showInputDialog();
	//------------------------------------
	//------- Begin pop up - MUST BE IN THIS SCOPE
	bool pressedOK = false, pressedCANCEL = false;
	bool open = true;

	if (ImGui::BeginPopupModal("inputPopUp", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
#ifdef _DEBUG
		//std::cout << "Scene: Entered BeginPopupModal " << string(popupTitle) << endl;
		//std::cout << "Window pop up appearing? " << ImGui::IsWindowAppearing() << endl;
#endif // _DEBUG

		ImGui::InputText("Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));

		// TODO: validate name

		ImGui::InputFloat3("Position", posBuffer);

		// Validate position input
		bool arePositionValuesValid = true;
		for (int i = 0; i < 3; ++i) {
			arePositionValuesValid = arePositionValuesValid &&
				!std::isnan(posBuffer[i]) &&
				!std::isinf(posBuffer[i]);
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
			// Close the popup 
			pressedCANCEL = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}


		// Check if OK button was pressed in the popup
		if (showModelDialog && pressedOK) {
			auto currModel = models.back();	// Surely loaded new model
			currModel->setName(nameBuffer);
			
			//TODO: translate the model to given position;

			// Reset the flag
			pressedOK = false;	// Reset flag
			showModelDialog = false; // Reset flag

		}
		else if (showModelDialog && pressedCANCEL && !pressedOK)
		{	// Model not added
			models.pop_back();	
			pressedCANCEL = false;
			showModelDialog = false;
		}

		if (showCamDialog && pressedOK)
		{
			//if(cameras.capacity())
			//{ 
				auto currCamera = cameras.back();	// Surely loaded new model
				currCamera->setName(nameBuffer);
			//}
			pressedOK = false;	// Reset flag
			showCamDialog = false; // Reset flag

		}
		else if (showCamDialog && pressedCANCEL && !pressedOK)
		{	// Model not added
			cameras.pop_back();
			pressedCANCEL = false;
			showCamDialog = false;
		}

	

	//--------------------------------------------------------------
	//------ Draw the ImGui::Image (Used for displaying our texture)
		
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	if (ImGui::Begin("Main Window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | \
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | \
		ImGuiWindowFlags_NoTitleBar ))
	{
		// Display the texture in ImGui:
		ImGui::Image((void*)(intptr_t)(m_renderer->m_textureID), viewport->WorkSize);
	ImGui::End();
	}

}

