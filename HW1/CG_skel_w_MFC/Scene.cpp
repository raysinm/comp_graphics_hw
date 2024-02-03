#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "PrimMeshModel.h"
#include "CG_skel_w_glfw.h"
#include <string>
#include <math.h>

using namespace std;
static char nameBuffer[64] = { 0 };
static float posBuffer[3] = { 0 };
bool add_showModelDlg = false, add_showCamDlg = false;

// TODO: Decide on ranges for transformations, projection
float FOV_RANGE_MIN = 0.01, FOV_RANGE_MAX = 89.99;
float ASPECT_RANGE_MIN = -10, ASPECT_RANGE_MAX = 10;
float PROJ_RANGE_MIN = -10, PROJ_RANGE_MAX = 10;

float TRNSL_RANGE_MIN = -10, TRNSL_RANGE_MAX = 10;
float ROT_RANGE_MIN = 0, ROT_RANGE_MAX = 360;
float SCALE_RANGE_MIN = -10, SCALE_RANGE_MAX = 10;


//--------------------------------------------------
//-------------------- CAMERA ----------------------
//--------------------------------------------------

Camera::Camera()
{
	// Default camera projection - Perspective
	Frustum(-1, 1, -1, 1, 1, 15);
	name = CAMERA_DEFAULT_NAME;
}

void Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up)
{
	//TODO: Implement
}

void Camera::LookAt(const Model* target)
{
	//This function is called from keyboard manager (pressed F to set focus)
	//TODO: Implement
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

	m_left = left; m_right = right; m_top = top; m_bottom = bottom;
	m_zNear = zNear; m_zFar = zFar;

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
	const float zNear, const float zFar)
{
	// Sets user-requested frostum	
	if (zNear >= zFar)
	{
		// Illegal	//TODO
		//Yonatan: Maybe switch between zNear and zFar? Or display an error messaged and return here.
	}

	m_left = left; m_right = right; m_top = top; m_bottom = bottom;
	m_zNear = zNear; m_zFar = zFar;


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
	m_fovy = fovy; m_aspect = aspect; m_zNear = zNear; m_zFar = zFar;
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

//--------------------------------------------------
//-------------------- SCENE ----------------------
//--------------------------------------------------

void Scene::AddCamera()
{
	//TODO: Add camera using the input x y z ...


	Camera* cam = new Camera();
	cameras.push_back(cam);
	string s = cam->getName();
	s += " " + std::to_string(cameras.size());
	cam->setName(s);

}

void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::ResetPopUpFlags()
{
	GUI_popup_pressedOK = false;		// Reset flag
	GUI_popup_pressedCANCEL = false;		// Reset flag
	add_showCamDlg = false;		// Reset flag
	add_showModelDlg = false;
	memset(nameBuffer, 0, IM_ARRAYSIZE(nameBuffer));
	memset(posBuffer, 0, sizeof(float)*3);
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


		//4. TODO: Add camera ' + ' signs. 
		//5. TODO: Add normals arrow lines (if enabled)

		//6. Rasterazation

		//This should hold the MeshModel vertices AFTER all Transforms and projection.
		//values: [-1, 1]
		vec2* vertecies = ((MeshModel*)model)->Get2dBuffer();
		// DEBUG
		//for (int i=0;i< ((MeshModel*)model)->Get2dBuffer_len(); i++)
		//{
		//	cout << vertecies[i];
		//}
		if(vertecies)
			m_renderer->SetBufferOfModel(vertecies, ((MeshModel*)model)->Get2dBuffer_len());
	}
	m_renderer->updateTexture();




	//5. Update the texture. (OpenGL stuff)
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
			if (ImGui::MenuItem("Model (.obj file)"))	// Loading Model
				{
					CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("(*.obj)|*.obj|All Files (*.*)|*.*||"));
					if (dlg.DoModal() == IDOK)
					{
						std::string filename((LPCTSTR)dlg.GetPathName());
						loadOBJModel(filename);
						add_showModelDlg = true;
						activeModel = models.size()-1;

					}
				}
			if (ImGui::BeginMenu("Prim Model"))	// Creating PrimModel of many types
			{
				if (ImGui::MenuItem("Cube"))
				{
					Cube* cube = new Cube();
					models.push_back(cube);
					activeModel = models.size() - 1;
					add_showModelDlg = true;
				}
				ImGui::EndMenu();

			}
			if (ImGui::MenuItem("Camera"))
			{
				AddCamera();
				add_showCamDlg = true;
				strcpy(nameBuffer, cameras[cameras.size() - 1]->getName().c_str());
			}
			
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Select"))
		{
			if (models.size() > 0)
			{
				if (ImGui::BeginMenu("Model"))
				{
					int len = models.size();
					for (int c = 0; c < len; c++)
					{
						if (ImGui::MenuItem(models[c]->getName().c_str(), NULL, &models[c]->selected))
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
			}
			if (ImGui::BeginMenu("Camera"))
			{
				int len = cameras.size();
				for (int c = 0; c < len; c++)
				{
					if (ImGui::MenuItem(cameras[c]->getName().c_str(), NULL, &cameras[c]->selected))
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


	//---------------------------------------------------------
	//------- Show Transformations Dialog - version 1 ---------
	//---------------------------------------------------------
	if (models.capacity() > 0)	// TODO: Use flags
	{

	if (ImGui::Begin("Transformations Window"))
	{
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		const char* names[2] = { "Camera", "Model" };
		if (ImGui::BeginTabBar("TransBar", tab_bar_flags))
		{
			ImGui::PushItemWidth(80);
			for (int n = 0; n < ARRAYSIZE(names); n++)
			{
				if (ImGui::BeginTabItem(names[n], 0, tab_bar_flags))
				{
					const char* name = names[n];
					vec4* g_rot;
					vec4* g_trnsl;

					if (!strcmp(names[n], "Camera"))
					{
						float* g_left = &(cameras[activeCamera]->m_left);
						float* g_right = &(cameras[activeCamera]->m_right);
						float* g_top = &(cameras[activeCamera]->m_top);
						float* g_bottom = &(cameras[activeCamera]->m_bottom);
						float* g_zNear = &(cameras[activeCamera]->m_zNear);
						float* g_zFar = &(cameras[activeCamera]->m_zFar);
						float* g_fovy = &(cameras[activeCamera]->m_fovy);
						float* g_aspect = &(cameras[activeCamera]->m_aspect);

						//float* g_rotX = &cameras[activeCamera]->m_rot.x;
						//float* g_rotY = &cameras[activeCamera]->m_rot.y;
						//float* g_rotZ = &cameras[activeCamera]->m_rot.z;
						g_rot = &(cameras[activeCamera]->m_rot);
						g_trnsl = &(cameras[activeCamera]->m_trnsl);


						ImGui::SeparatorText("Projection");
						//ImGui::DragFloat("drag float3", vec4f, 0.01f, 0.0f, 1.0f);
						ImGui::SliderFloat("Left", g_left, -10.0f, 10.0f, "Left"); ImGui::SameLine();
						ImGui::SliderFloat("Right", g_right, -10.0f, 10.0f, "Right"); ImGui::SameLine();
						ImGui::SliderFloat("Top", g_top, -10.0f, 10.0f, "Top"); ImGui::SameLine();
						ImGui::SliderFloat("Bottom", g_bottom, -10.0f, 10.0f, "Bottom");
						ImGui::SliderFloat("zNear", g_zNear, -10.0f, 10.0f, "zNear"); ImGui::SameLine();
						ImGui::SliderFloat("zFar", g_zFar, -10.0f, 10.0f, "zFar");
						ImGui::SliderFloat("FovY", g_fovy, FOV_RANGE_MIN, FOV_RANGE_MAX, "FovY"); ImGui::SameLine();
						ImGui::SliderFloat("Aspect", g_aspect, -10.0f, 10.0f, "Aspect");

					}
					else if (!strcmp(names[n], "Model"))
					{
						g_rot = &((MeshModel*)models[activeModel])->_rot;
						g_trnsl = &((MeshModel*)models[activeModel])->_trnsl;
					}
					ImGui::SeparatorText("Transformation");
					ImGui::Text("Translation");
					ImGui::SliderFloat("X", &(g_trnsl->x), -10.0f, 10.0f, "X"); ImGui::SameLine();
					ImGui::SliderFloat("Y", &(g_trnsl->y), -10.0f, 10.0f, "Y"); ImGui::SameLine();
					ImGui::SliderFloat("Z", &(g_trnsl->z), -10.0f, 10.0f, "Z");
					ImGui::Text("Rotation");
					ImGui::SliderFloat("Rotate X", &(g_rot->x), ROT_RANGE_MIN, ROT_RANGE_MAX, "Rotate X"); ImGui::SameLine();
					ImGui::SliderFloat("Rotate Y", &(g_rot->y), ROT_RANGE_MIN, ROT_RANGE_MAX, "Rotate Y"); ImGui::SameLine();
					ImGui::SliderFloat("Rotate Z", &(g_rot->z), ROT_RANGE_MIN, ROT_RANGE_MAX, "Rotate Z");
					if (!strcmp(names[n], "Model"))
					{
						vec4* g_scale = &((MeshModel*)models[activeModel])->_scale;
						ImGui::Text("Scale");
						ImGui::SliderFloat("Scale X", &(g_scale->x), -10.0f, 10.0f, "Scale X"); ImGui::SameLine();
						ImGui::SliderFloat("Scale Y", &(g_scale->y), -10.0f, 10.0f, "Scale Y"); ImGui::SameLine();
						ImGui::SliderFloat("Scale Z", &(g_scale->z), -10.0f, 10.0f, "Scale Z");

						// World transformations
						ImGui::SeparatorText("World Transformation");

						g_trnsl = &((MeshModel*)models[activeModel])->_trnsl_w;
						g_rot = &((MeshModel*)models[activeModel])->_rot_w;
						g_scale = &((MeshModel*)models[activeModel])->_scale_w;

						ImGui::Text("Translation");
						ImGui::SliderFloat("W X", &(g_trnsl->x), -10.0f, 10.0f, "X"); ImGui::SameLine();
						ImGui::SliderFloat("W Y", &(g_trnsl->y), -10.0f, 10.0f, "Y"); ImGui::SameLine();
						ImGui::SliderFloat("W Z", &(g_trnsl->z), -10.0f, 10.0f, "Z");
						ImGui::Text("Rotation");
						ImGui::SliderFloat("W Rotate X", &(g_rot->x), ROT_RANGE_MIN, ROT_RANGE_MAX, "Rotate X"); ImGui::SameLine();
						ImGui::SliderFloat("W Rotate Y", &(g_rot->y), ROT_RANGE_MIN, ROT_RANGE_MAX, "Rotate Y"); ImGui::SameLine();
						ImGui::SliderFloat("W Rotate Z", &(g_rot->z), ROT_RANGE_MIN, ROT_RANGE_MAX, "Rotate Z");
						ImGui::Text("Scale");
						ImGui::SliderFloat("W Scale X", &(g_scale->x), -10.0f, 10.0f, "Scale X"); ImGui::SameLine();
						ImGui::SliderFloat("W Scale Y", &(g_scale->y), -10.0f, 10.0f, "Scale Y"); ImGui::SameLine();
						ImGui::SliderFloat("W Scale Z", &(g_scale->z), -10.0f, 10.0f, "Scale Z");

					}
					ImGui::EndTabItem();
				}
			}
			ImGui::PopItemWidth();

			ImGui::EndTabBar();
		}
		ImGui::End();
	}
	}


	//Check if the popup should be shown
	if (add_showModelDlg || add_showCamDlg)
	{
		ImGui::OpenPopup(ADD_INPUT_POPUP_TITLE);
	}

	

	//---------------------------------------------------
	//------- Begin pop up - MUST BE IN THIS SCOPE ------
	//---------------------------------------------------
	bool open_popup_AddObject = true; //Must be here unless it won't work... (Weird ImGui stuff i guess)
	if (ImGui::BeginPopupModal(ADD_INPUT_POPUP_TITLE, 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::InputText("Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
		ImGui::InputFloat3("Position (x,y,z)", posBuffer);

		/* Validate name (not empty + not in use) */
		bool isNameValid = strlen(nameBuffer) > 0;
		if (add_showCamDlg)
		{
			for (auto i : cameras)
			{
				if (i != cameras.back())
				{
					isNameValid = isNameValid && strcmp(i->getName().c_str(), nameBuffer);
				}
			}
		}
		else if (add_showModelDlg)
		{
			for (auto i : models)
			{
				if (i != models.back())
				{
					isNameValid = isNameValid && strcmp(i->getName().c_str(), nameBuffer);
				}
			}
		}

		/* Validate position input */
		bool arePositionValuesValid = true;
		for (int i = 0; i < 3; ++i)
		{
			arePositionValuesValid = arePositionValuesValid && !std::isnan(posBuffer[i]) && !std::isinf(posBuffer[i]);
		}


		// Notify the user if position values are not in the specified range
		if (!arePositionValuesValid)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Position values must be float");
		}
		else if (!isNameValid)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Name is not valid (or in-use)");
		}

		// Add buttons for OK and Cancel
		if (ImGui::Button("OK") && arePositionValuesValid && isNameValid)
		{
			GUI_popup_pressedOK = true;
			GUI_popup_pressedCANCEL = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			GUI_popup_pressedOK = false;
			GUI_popup_pressedCANCEL = true;
			ImGui::CloseCurrentPopup();
		}


		ImGui::EndPopup();
	}

	//------------------------------------
	//--------  Handle pop-ups -----------
	if (add_showModelDlg)
	{
		if (GUI_popup_pressedOK)
		{
			auto currModel = models.back();	// Surely loaded new model
			currModel->setName(nameBuffer);

			//TODO: translate the model to given position;

			ResetPopUpFlags();


		}
		else if ( GUI_popup_pressedCANCEL )
		{
			delete ((MeshModel*)models[models.size() - 1]);
			models.pop_back();
			
			ResetPopUpFlags();
		}
	}
	if (add_showCamDlg)
	{
		if (GUI_popup_pressedOK)
		{
			auto currCamera = cameras.back();
			currCamera->setName(nameBuffer);
			
			//TODO: translate the camera to given position;
			
			ResetPopUpFlags();
		}
		else if (GUI_popup_pressedCANCEL)
		{
			delete cameras[cameras.size() - 1];
			cameras.pop_back();

			ResetPopUpFlags();
		}
	}

	

	//--------------------------------------------------------------
	//------ Draw the ImGui::Image (Used for displaying our texture)
 	//--------------------------------------------------------------
		
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

Camera* Scene::GetActiveCamera()
{
	return (activeCamera == NOT_SELECTED ? nullptr : cameras[activeCamera]);
}

Model* Scene::GetActiveModel()
{
	return (activeModel == NOT_SELECTED ? nullptr : models[activeModel]);
}

