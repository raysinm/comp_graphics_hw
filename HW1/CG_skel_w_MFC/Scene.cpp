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
bool showTransWindow = false;

// TODO: Decide on ranges for transformations, projection
const float FOV_RANGE_MIN = 0.01, FOV_RANGE_MAX = 89.99;
const float ASPECT_RANGE_MIN = -10, ASPECT_RANGE_MAX = 10;
const float PROJ_RANGE_MIN = -10, PROJ_RANGE_MAX = 10;

const float TRNSL_RANGE_MIN = -10, TRNSL_RANGE_MAX = 10;
const float ROT_RANGE_MIN = 0, ROT_RANGE_MAX = 360;
const float SCALE_RANGE_MIN = -10, SCALE_RANGE_MAX = 10;


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
	cam->setName(s); //Camera 1, Camera 2, Camera 3 ...

}

void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::ResetPopUpFlags()
{
	GUI_popup_pressedOK = false;		// Reset flag
	GUI_popup_pressedCANCEL = false;	// Reset flag
	add_showCamDlg = false;				// Reset flag
	add_showModelDlg = false;			// Reset flag
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
		//Don't draw new model before user clicked 'OK'.
		if (!model->GetUserInitFinished())
			continue;

		//model->draw(cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);

		//3.5 Projection
		// Uses camera 
		// projection =Ortho * vertex
		// normalized_projection = ST * projection


		//4. TODO: Add camera ' + ' signs. 
		//5. TODO: Add normals arrow lines (if enabled)


		//values: [-1, 1]
		//vec2* vertecies = ((MeshModel*)model)->Get2dBuffer();
		//unsigned int len = ((MeshModel*)model)->Get2dBuffer_len();
		//if(vertecies)
		//	m_renderer->SetBufferOfModel(vertecies, len);
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
	style.SeparatorTextBorderSize = 10;
	style.SeparatorTextAlign= ImVec2(0.5, 0.5);
	style.SeparatorTextPadding = ImVec2(20, 10);

	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	if (!showTransWindow && activeModel != NOT_SELECTED) //UnSelect the object if it's transformation windows closed
	{
		models[activeModel]->selected = false;
		activeModel = false;
	}
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
						showTransWindow = true;
					}
				}
			if (ImGui::BeginMenu("Primitive Model"))	// Creating PrimModel of many types
			{
				if (ImGui::MenuItem("Cube"))
				{
					Cube* cube = new Cube();
					models.push_back(cube);

					cube->setName("Cube");
					strcpy(nameBuffer, cube->getName().c_str());
					add_showModelDlg = true;
					showTransWindow = true;
				}
				ImGui::EndMenu();

			}
			if (ImGui::MenuItem("Camera"))
			{
				AddCamera();
				strcpy(nameBuffer, cameras[cameras.size() - 1]->getName().c_str());
				add_showCamDlg = true;
				showTransWindow = true;
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
							activeModel = c;
							UpdateModelSelection();

							showTransWindow = true;
							cout << "(debug) active model: " << activeModel << endl;
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
							cameras[t]->selected = false;

						/* Select current camera */
						activeCamera = c;
						cameras[c]->selected = true;
						showTransWindow = true;
						cout << "(debug) active Camera: " << activeCamera<< endl;
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
	if (activeCamera != NOT_SELECTED && !add_showModelDlg && !add_showCamDlg && showTransWindow)
	{
		if (ImGui::Begin("Transformations Window", &showTransWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
#define MODEL_TAB_INDEX  0
#define CAMERA_TAB_INDEX 1
			const char* names[2] = { 0 };
			names[CAMERA_TAB_INDEX] = "Camera";
			names[MODEL_TAB_INDEX] = "Model";

			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("TransBar", tab_bar_flags))
			{
				ImGui::PushItemWidth(80);
				for (int n = 0; n < ARRAYSIZE(names); n++)
				{
					if (activeModel == NOT_SELECTED && n == MODEL_TAB_INDEX)
						continue;
					MeshModel* activeMesh = nullptr;
					if (ImGui::BeginTabItem(names[n], 0, tab_bar_flags))
					{
						const char* name = names[n];
						vec4* g_trnsl = nullptr;
						vec4* g_rot   = nullptr;

						if (n == CAMERA_TAB_INDEX)
						{
							string name("Name: ");
							name += cameras[activeCamera]->getName();
							ImGui::Text(name.c_str());

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

							ImGui::SliderFloat("Left", g_left,		PROJ_RANGE_MIN, PROJ_RANGE_MAX, "%d"); ImGui::SameLine();
							ImGui::SliderFloat("Right", g_right,	PROJ_RANGE_MIN, PROJ_RANGE_MAX, "%d"); ImGui::SameLine();
							ImGui::SliderFloat("Top", g_top,		PROJ_RANGE_MIN, PROJ_RANGE_MAX, "%d"); ImGui::SameLine();
							ImGui::SliderFloat("Bottom", g_bottom,	PROJ_RANGE_MIN, PROJ_RANGE_MAX, "%d");
							ImGui::SliderFloat("zNear", g_zNear,	PROJ_RANGE_MIN, PROJ_RANGE_MAX, "%d"); ImGui::SameLine();
							ImGui::SliderFloat("zFar", g_zFar,		PROJ_RANGE_MIN, PROJ_RANGE_MAX, "%d");

							ImGui::SliderFloat("FovY", g_fovy,		FOV_RANGE_MIN, FOV_RANGE_MAX,		"%.1f"); ImGui::SameLine();
							ImGui::SliderFloat("Aspect", g_aspect,	ASPECT_RANGE_MIN, ASPECT_RANGE_MAX, "%.1f");

						}
						else if (n == MODEL_TAB_INDEX)
						{
							activeMesh = (MeshModel*)models[activeModel];
							string name("Name: ");
							name += models[activeModel]->getName();
							ImGui::Text(name.c_str());

							g_trnsl = &(activeMesh->_trnsl);
							g_rot   = &(activeMesh->_rot);
							bool* dispFaceNormal   = &(activeMesh->showFaceNormals);
							bool* dispVertexNormal = &(activeMesh->showVertexNormals);
							
							ImGui::Checkbox("Display Face Normals  ", dispFaceNormal);
							ImGui::Checkbox("Display Vertex Normals", dispVertexNormal);
						}
						
						ImGui::SeparatorText("Model space");

						ImGui::Text("Translation (X Y Z)");
						ImGui::SliderFloat("##X_MT", &(g_trnsl->x), TRNSL_RANGE_MIN, TRNSL_RANGE_MAX); ImGui::SameLine();
						ImGui::SliderFloat("##Y_MT", &(g_trnsl->y), TRNSL_RANGE_MIN, TRNSL_RANGE_MAX); ImGui::SameLine();
						ImGui::SliderFloat("##Z_MT", &(g_trnsl->z), TRNSL_RANGE_MIN, TRNSL_RANGE_MAX); ImGui::SameLine();
						if (ImGui::Button("reset##MT"))
						{
							if (n == CAMERA_TAB_INDEX)
							{
								cameras[activeCamera]->ResetTranslation();
							}
							else if (n == MODEL_TAB_INDEX)
							{
								activeMesh->ResetUserTransform_translate_model();
							}
						}

						ImGui::Text("Rotation (X Y Z)");
						ImGui::SliderFloat("##X_MR", &(g_rot->x), ROT_RANGE_MIN, ROT_RANGE_MAX, "%.0f"); ImGui::SameLine();
						ImGui::SliderFloat("##Y_MR", &(g_rot->y), ROT_RANGE_MIN, ROT_RANGE_MAX, "%.0f"); ImGui::SameLine();
						ImGui::SliderFloat("##Z_MR", &(g_rot->z), ROT_RANGE_MIN, ROT_RANGE_MAX, "%.0f"); ImGui::SameLine();
						if (ImGui::Button("reset##MR"))
						{
							if (n == CAMERA_TAB_INDEX)
							{
								cameras[activeCamera]->ResetRotation();
							}
							else if (n == MODEL_TAB_INDEX)
							{
								activeMesh->ResetUserTransform_rotate_model();
							}
						}
						
						if (n == MODEL_TAB_INDEX)
						{
							vec4* g_scale = &(activeMesh->_scale);

							ImGui::Text("Scale (X Y Z)");
							ImGui::SliderFloat("##X_MS", &(g_scale->x), SCALE_RANGE_MIN, SCALE_RANGE_MAX, "%.1f"); ImGui::SameLine();
							ImGui::SliderFloat("##Y_MS", &(g_scale->y), SCALE_RANGE_MIN, SCALE_RANGE_MAX, "%.1f"); ImGui::SameLine();
							ImGui::SliderFloat("##Z_MS", &(g_scale->z), SCALE_RANGE_MIN, SCALE_RANGE_MAX, "%.1f"); ImGui::SameLine();
							if (ImGui::Button("reset##MS"))
							{
									activeMesh->ResetUserTransform_scale_model();
							}
							
							
							// World transformations
							ImGui::SeparatorText("World space");

							vec4* trnsl_w = &(activeMesh->_trnsl_w);
							vec4* rot_w   = &(activeMesh->_rot_w);
							vec4* scale_w = &(activeMesh->_scale_w);

							ImGui::Text("Translation (X Y Z)");
							ImGui::SliderFloat("##X_WT", &(trnsl_w->x), TRNSL_RANGE_MIN, TRNSL_RANGE_MAX, "%.0f"); ImGui::SameLine();
							ImGui::SliderFloat("##Y_WT", &(trnsl_w->y), TRNSL_RANGE_MIN, TRNSL_RANGE_MAX, "%.0f"); ImGui::SameLine();
							ImGui::SliderFloat("##Z_WT", &(trnsl_w->z), TRNSL_RANGE_MIN, TRNSL_RANGE_MAX, "%.0f"); ImGui::SameLine();
							if (ImGui::Button("reset##WT"))
							{
								activeMesh->ResetUserTransform_translate_world();
							}



							ImGui::Text("Rotation (X Y Z)");
							ImGui::SliderFloat("##X_WR", &(rot_w->x), ROT_RANGE_MIN, ROT_RANGE_MAX, "%.0f"); ImGui::SameLine();
							ImGui::SliderFloat("##Y_WR", &(rot_w->y), ROT_RANGE_MIN, ROT_RANGE_MAX, "%.0f"); ImGui::SameLine();
							ImGui::SliderFloat("##Z_WR", &(rot_w->z), ROT_RANGE_MIN, ROT_RANGE_MAX, "%.0f"); ImGui::SameLine();
							if (ImGui::Button("reset##WR"))
							{
								activeMesh->ResetUserTransform_rotate_world();
							}


							ImGui::Text("Scale (X Y Z)");
							ImGui::SliderFloat("##X_WS", &(scale_w->x), SCALE_RANGE_MIN, SCALE_RANGE_MAX, "%.1f"); ImGui::SameLine();
							ImGui::SliderFloat("##Y_WS", &(scale_w->y), SCALE_RANGE_MIN, SCALE_RANGE_MAX, "%.1f"); ImGui::SameLine();
							ImGui::SliderFloat("##Z_WS", &(scale_w->z), SCALE_RANGE_MIN, SCALE_RANGE_MAX, "%.1f"); ImGui::SameLine();
							if (ImGui::Button("reset##WS"))
							{
								activeMesh->ResetUserTransform_scale_world();
							}
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
			((MeshModel*)currModel)->setTranslationWorld(vec3(posBuffer[0], posBuffer[1], posBuffer[2]));

			activeModel = models.size() - 1;
			UpdateModelSelection(); //Deselect all others & select activeModel.

			currModel->SetUserInitFinished();
			ResetPopUpFlags();


		}
		else if ( GUI_popup_pressedCANCEL )
		{
			activeModel = NOT_SELECTED;
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
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMouseInputs))
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

void Scene::UpdateModelSelection()
{
	int len = models.size();
	for (int i = 0; i < len; i++)
	{
		models[i]->selected = false;
	}
	models[activeModel]->selected = true;
}

