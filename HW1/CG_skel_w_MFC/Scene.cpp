#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "PrimMeshModel.h"
#include "CG_skel_w_glfw.h"
#include <string>
#include <math.h>

#define MODEL_TAB_INDEX  0
#define CAMERA_TAB_INDEX 1

using namespace std;
static char nameBuffer[64] = { 0 };
static float posBuffer[3] = { 0 };
bool add_showModelDlg = false, add_showCamDlg = false;
bool showTransWindow = false;
bool constScaleRatio = false;
bool constScaleRatio_w = false;
int transformationWindowWidth = 0;

// TODO: Decide on ranges for transformations, projection
const float FOV_RANGE_MIN = 0.001, FOV_RANGE_MAX = 89.999;
const float ASPECT_RANGE_MIN = -10, ASPECT_RANGE_MAX = 10;
const float PROJ_RANGE_MIN = -20, PROJ_RANGE_MAX = 20;
const float ZCAM_RANGE_MIN = 0.001, ZCAM_RANGE_MAX = 100;
//const float PROJ_RANGE_MIN = -20, PROJ_RANGE_MAX = 10;

const float TRNSL_RANGE_MIN = -40, TRNSL_RANGE_MAX = 40;
const float ROT_RANGE_MIN = -180, ROT_RANGE_MAX = 180;
const float SCALE_RANGE_MIN = -1, SCALE_RANGE_MAX = 20;


//--------------------------------------------------
//-------------------- CAMERA ----------------------
//--------------------------------------------------
Camera::Camera()
{
	name = CAMERA_DEFAULT_NAME;
	
	ResetRotation();
	ResetTranslation();
	resetProjection();
	setOrtho();
}

mat4 Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up)
{
	vec4 n = normalize(eye - at);
	vec4 u = normalize(cross(up, n));
	vec4 v = normalize(cross(n, u));
	
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 c = mat4(u, v, n, t);

	return c * Translate(-eye);

}

//This function is called from keyboard manager (pressed F to set focus)
void Camera::LookAt(const Model* target)
{
	mat4 total_rotation = RotateZ(c_rot.z) * (RotateY(c_rot.y) * RotateX(c_rot.x));
	vec4 up = total_rotation[1];

	this->target = ((MeshModel*)target)->getCenterOffMass();
	
	cTransform = LookAt(c_trnsl, this->target, up);
}

void Camera::LookAt()
{
	mat4 total_rotation = RotateZ(c_rot.z) * (RotateY(c_rot.y) * RotateX(c_rot.x));
	vec4 up = total_rotation[1];

	
	cTransform = LookAt(c_trnsl, this->target, up);
}


void Camera::setOrtho()
{
	GLfloat x = c_right - c_left;
	GLfloat y = c_top - c_bottom;
	GLfloat z = c_zFar - c_zNear;

	projection = mat4(  2 / x, 0    , 0      , -(c_left + c_right) / x,
							0, 2 / y, 0      , -(c_top + c_bottom) / y,
							0, 0    , -2 / z , -(c_zFar + c_zNear) / z,
							0, 0    , 0      , 1);
}

void Camera::setPerspective()
{
	GLfloat x = c_right - c_left;
	GLfloat y = c_top   - c_bottom;
	GLfloat z = c_zFar  - c_zNear;

	projection = mat4(2*c_zNear / x	, 0				, (c_right+c_left)   / x , 0					 ,
					  0				, 2*c_zNear / y	, (c_top + c_bottom) / y , 0				     ,
					  0				, 0				, -(c_zFar+c_zNear)  / z , -(2*c_zNear*c_zFar)/z ,
					  0				, 0				, -1				     , 0);

}

void Camera::setPerspectiveByFov()
{
	c_top = c_zNear * tan((M_PI / 180) * c_fovy);
	c_right = c_top * c_aspect;
	
	
	//c_bottom = -c_top;
	//c_left = -c_right;
	
	setPerspective();
}

void Camera::setPerspectiveByParams()
{
	float width  = (c_right - c_left);
	float height = (c_top - c_bottom);
	
	
	c_fovy = (180 / M_PI) * atanf(c_top / c_zNear);
	c_aspect = c_right / c_top;
	setPerspective();

}


void Camera::resetProjection()
{
	c_left = c_bottom = -DEF_PARAM;
	c_right = c_top = DEF_PARAM;
	c_zNear = DEF_ZNEAR;
	c_zFar = DEF_ZFAR;

	setPerspectiveByParams();

}

void Camera::updateTransform()
{
	mat4 rot_x = RotateX(c_rot.x);
	mat4 rot_y = RotateY(c_rot.y);
	mat4 rot_z = RotateZ(c_rot.z);
	mat4 trnsl = Translate(c_trnsl);

	cTransform = rot_z * rot_y * rot_x * trnsl ; // yaw pitch roll order
}


//--------------------------------------------------
//-------------------- SCENE ----------------------
//--------------------------------------------------

void Scene::AddCamera()
{
	Camera* cam = new Camera();
	cameras.push_back(cam);

	string s = cam->getName();
	s += " " + std::to_string(cameras.size());
	cam->setName(s); //Camera 1, Camera 2, Camera 3 ...

}

void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);

	/* Get the filename as the default name */
	string extractedName = MODEL_DEFAULT_NAME;
	unsigned int pos = fileName.find_last_of('\\');
	if (pos != std::string::npos)
	{
		extractedName = fileName.substr(pos + 1);
		pos = extractedName.find_last_of('.');
		if(pos != std::string::npos)
			extractedName = extractedName.substr(0, pos); //Cut the .obj in the end of the name
	}
	model->setName(extractedName);

	/* Add model to the models array */
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
					
	//2. Clear the pixel buffer before drawing new frame
	m_renderer->clearBuffer();

	//3. Update camera transformation matrix (cTransform)
	//cameras[activeCamera]->updateTransform();

	//4. draw each MeshModel
	for (auto model : models)
	{
		//Don't draw new model before user clicked 'OK'.
		if (!model->GetUserInitFinished())
			continue;

		model->draw(cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);

		//values: [-1, 1]
		vec2* vertecies = ((MeshModel*)model)->Get2dBuffer(MODEL);
		unsigned int len = ((MeshModel*)model)->Get2dBuffer_len(MODEL);
		if(vertecies)
			m_renderer->SetBufferOfModel(vertecies, len);
		
		// Bounding Box
		if (((MeshModel*)model)->showBoundingBox)
		{
			vec2* bbox_vertices = ((MeshModel*)model)->Get2dBuffer(BBOX);
			len = ((MeshModel*)model)->Get2dBuffer_len(BBOX);
			if (bbox_vertices)
				m_renderer->SetBufferOfModel(bbox_vertices, len, vec4(0,1,0,1));
		}
		
		// Vertex Normals
		if (((MeshModel*)model)->showVertexNormals)
		{
			vec2* v_norm_vertices = ((MeshModel*)model)->Get2dBuffer(V_NORMAL);
			len = ((MeshModel*)model)->Get2dBuffer_len(V_NORMAL);
			if (v_norm_vertices)
				m_renderer->SetBufferLines(v_norm_vertices, len, vec4(1,0,0));

		}
		
		// Face normals
		if (((MeshModel*)model)->showFaceNormals)
		{
			vec2* f_norm_vertices = ((MeshModel*)model)->Get2dBuffer(F_NORMAL);
			len = ((MeshModel*)model)->Get2dBuffer_len(F_NORMAL);
			if (f_norm_vertices)
				m_renderer->SetBufferLines(f_norm_vertices, len, vec4(0,0,1));
		}

	}
	
	//TODO: Add camera ' + ' signs. 

	//5. Update the texture. (OpenGL stuff)
	m_renderer->updateTexture();



}

bool closedTransfWindowFlag = false;
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
		activeModel = NOT_SELECTED;

	}
	if (!showTransWindow)
	{
		if (!closedTransfWindowFlag)
		{
			//Call manually the resize callback to update the scene:
			resize_callback_handle(m_renderer->GetWindowSize().x, m_renderer->GetWindowSize().y);
			closedTransfWindowFlag = true;
			transformationWindowWidth = 0;
		}
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

						strcpy(nameBuffer, models.back()->getName().c_str());
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

					strcpy(nameBuffer, cube->getName().c_str());
					add_showModelDlg = true;
					showTransWindow = true;
				}
				if (ImGui::MenuItem("Pyramid (triangular)"))
				{
					TriPyramid* tri_pyr = new TriPyramid();
					models.push_back(tri_pyr);

					strcpy(nameBuffer, tri_pyr->getName().c_str());
					add_showModelDlg = true;
					showTransWindow = true;
				}
				ImGui::EndMenu();

			}
			if (ImGui::MenuItem("Camera"))
			{
				AddCamera();
				strcpy(nameBuffer, cameras.back()->getName().c_str());
				vec4 c_trnsl = cameras.back()->getTranslation();
				posBuffer[0] = c_trnsl.x;
				posBuffer[1] = c_trnsl.y;
				posBuffer[2] = c_trnsl.z;

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
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu(); //End select
		}
		
		// Delete Model/Camera
		if (models.size() > 0 || cameras.size() > 1)
		{
			if (ImGui::BeginMenu("Delete..."))
			{
				if (models.size() > 0)
				{
					if (ImGui::BeginMenu("Model"))
					{
						for (int c = 0; c < models.size(); c++)
						{
							if (ImGui::MenuItem(models[c]->getName().c_str(), NULL))
							{
								models.erase(models.begin() + c);
								if (c == activeModel)
								{
									activeModel = NOT_SELECTED;	// Selected model deleted
								}
								else if (activeModel > c)
								{
									--activeModel;	// index moved
								}
							}
						}
						ImGui::EndMenu();
					}
				}
				if (cameras.size() > 1)	// Delete only if there is more than one camera
				{
					if (ImGui::BeginMenu("Camera"))
					{
						for (int c = 0; c < cameras.size(); c++)
						{
							if (ImGui::MenuItem(cameras[c]->getName().c_str(), NULL))
							{
								/* Delete current camera */
								cameras.erase(cameras.begin() + c);

								if (c == activeCamera)
								{
									activeCamera = max(0, activeCamera - 1);
								}
								else if (activeCamera > c)
								{
									--activeCamera;	// index changed
								}
								cameras[activeCamera]->selected = true;

							}
						}
						ImGui::EndMenu();
					}

				}

				ImGui::EndMenu(); //End delete
			}
		}


		ImGui::EndMainMenuBar();
	}


	//---------------------------------------------------------
	//------------ Transformations Window ---------------------
	//---------------------------------------------------------
	if (activeCamera != NOT_SELECTED && !add_showModelDlg && !add_showCamDlg && showTransWindow)
	{
		closedTransfWindowFlag = false;
		float mainMenuBarHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImGui::SetNextWindowPos(ImVec2(0, mainMenuBarHeight), ImGuiCond_Always);
		ImGui::SetNextWindowSizeConstraints(ImVec2(310, m_renderer->GetWindowSize().y - mainMenuBarHeight),\
											ImVec2(m_renderer->GetWindowSize().x / 2, m_renderer->GetWindowSize().y - mainMenuBarHeight));
		if (ImGui::Begin("Transformations Window", &showTransWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
		{
			int currentWidth = (int)ImGui::GetWindowSize().x;
			if (currentWidth != transformationWindowWidth)
			{
				transformationWindowWidth = currentWidth;
				resize_callback_handle(m_renderer->GetWindowSize().x, m_renderer->GetWindowSize().y);
			}

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
						vec4 prev_trnsl;
						vec4 prev_rot;

						if (n == CAMERA_TAB_INDEX)
						{
							string name("Name: ");
							name += cameras[activeCamera]->getName();
							ImGui::Text(name.c_str());

							float* g_left = &(cameras[activeCamera]->c_left);
							float* g_right = &(cameras[activeCamera]->c_right);
							float* g_top = &(cameras[activeCamera]->c_top);
							float* g_bottom = &(cameras[activeCamera]->c_bottom);
							float* g_zNear = &(cameras[activeCamera]->c_zNear);
							float* g_zFar = &(cameras[activeCamera]->c_zFar);
							float* g_fovy = &(cameras[activeCamera]->c_fovy);
							float* g_aspect = &(cameras[activeCamera]->c_aspect);
							g_rot = &(cameras[activeCamera]->c_rot);
							g_trnsl = &(cameras[activeCamera]->c_trnsl);
								
							static int g_ortho = 1;
							float prev_left = *g_left;
							float prev_right = *g_right;
							float prev_bottom = *g_bottom;
							float prev_top = *g_top;
							float prev_zNear = *g_zNear;
							float prev_zFar = *g_zFar;

							ImGui::SeparatorText("Projection");
							ImGui::RadioButton("Orthographic", &g_ortho, 1); ImGui::SameLine();
							ImGui::RadioButton("Perspective", &g_ortho, 0);


							ImGui::DragFloat("##Left_CP", g_left, 0.01f, 0, 0, "Left = %.1f "); ImGui::SameLine();
							ImGui::DragFloat("##Right_CP", g_right, 0.01f, 0, 0, "Right = %.1f ");


							ImGui::DragFloat("##Top_CP", g_top, 0.01f, 0, 0, "Top = %.1f "); ImGui::SameLine();
							ImGui::DragFloat("##Bottom_CP", g_bottom, 0.01f, 0, 0, "Bot = %.1f ");


							ImGui::DragFloat("##zNear_CP", g_zNear, 0.01f, 0, 0, "z-Near = %.1f "); ImGui::SameLine();
							ImGui::DragFloat("##zFar_CP", g_zFar, 0.01f, 0, 0, "z-Far = %.1f ");

							// Set Camera projection type
							if (g_ortho == 1)
							{
								if (prev_left   != *g_left   || prev_right != *g_right ||
									prev_bottom != *g_bottom || prev_top   != *g_top   ||
									prev_zNear  != *g_zNear  || prev_zFar  != *g_zFar)
								{
									cameras[activeCamera]->setOrtho();
								}
							}
							else
							{
								float prev_fovy = *g_fovy;
								float prev_aspect = *g_aspect;

								ImGui::DragFloat("##FovY", g_fovy, 0.01f, FOV_RANGE_MIN, FOV_RANGE_MAX, "FovY = %.1f "); ImGui::SameLine();
								ImGui::DragFloat("##Aspect", g_aspect, 0.01f, ASPECT_RANGE_MIN, ASPECT_RANGE_MAX, "A / R = %.1f ");

								if (prev_fovy != *g_fovy || prev_aspect != *g_aspect) //User changed FOV or Aspect Ratio
								{
									cameras[activeCamera]->setPerspectiveByFov();
								}
								else if (prev_left   != *g_left   || prev_right != *g_right ||
										 prev_bottom != *g_bottom || prev_top   != *g_top   ||
										 prev_zNear  != *g_zNear  || prev_zFar  != *g_zFar)
								{
									cameras[activeCamera]->setPerspectiveByParams();
								}

							}
							if (ImGui::Button("reset"))
							{
								cameras[activeCamera]->resetProjection();
							}


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
							bool* dispBoundingBox = &(activeMesh->showBoundingBox);

							ImGui::Checkbox("Display Face Normals  ", dispFaceNormal);
							ImGui::Checkbox("Display Vertex Normals", dispVertexNormal);
							ImGui::Checkbox("Display Bounding Box", dispBoundingBox);
						}
						
						string sep_text = n == MODEL_TAB_INDEX ? "Model space" : "Camera space";
						ImGui::SeparatorText(sep_text.c_str());

						if (n == CAMERA_TAB_INDEX)
						{
							prev_trnsl = *g_trnsl;
							prev_rot = *g_rot;
						}
						ImGui::Text("Translation (X Y Z)");
						ImGui::DragFloat("##X_MT", &(g_trnsl->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
						ImGui::DragFloat("##Y_MT", &(g_trnsl->y), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
						ImGui::DragFloat("##Z_MT", &(g_trnsl->z), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
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
						ImGui::DragFloat("##X_MR", &(g_rot->x), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
						ImGui::DragFloat("##Y_MR", &(g_rot->y), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
						ImGui::DragFloat("##Z_MR", &(g_rot->z), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
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
						
						if (n == CAMERA_TAB_INDEX)
						{
							if (prev_trnsl != *g_trnsl || prev_rot != *g_rot)
							{
								cameras[activeCamera]->LookAt();
							}
						}


						if (n == MODEL_TAB_INDEX)
						{
							vec4* g_scale = &(activeMesh->_scale);

							ImGui::Text("Scale (X Y Z)");
							ImGui::Checkbox("keep ratio", &constScaleRatio);
							ImGui::DragFloat("##X_MS", &(g_scale->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
							if (constScaleRatio)
							{
								g_scale->y = g_scale->z = g_scale->x;
							}
							else
							{
								ImGui::DragFloat("##Y_MS", &(g_scale->y),0.01f, 0, 0, "%.1f"); ImGui::SameLine();
								ImGui::DragFloat("##Z_MS", &(g_scale->z),0.01f, 0, 0, "%.1f"); ImGui::SameLine();
							}
							
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
							ImGui::DragFloat("##X_WT", &(trnsl_w->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
							ImGui::DragFloat("##Y_WT", &(trnsl_w->y), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
							ImGui::DragFloat("##Z_WT", &(trnsl_w->z), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();

							// Handle keyboard navigation for FloatSliders
							if (ImGui::Button("reset##WT"))
							{
								activeMesh->ResetUserTransform_translate_world();
							}



							ImGui::Text("Rotation (X Y Z)");
							ImGui::DragFloat("##X_WR", &(rot_w->x), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
							ImGui::DragFloat("##Y_WR", &(rot_w->y), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
							ImGui::DragFloat("##Z_WR", &(rot_w->z), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
							if (ImGui::Button("reset##WR"))
							{
								activeMesh->ResetUserTransform_rotate_world();
							}


							ImGui::Text("Scale (X Y Z)");
							ImGui::Checkbox("keep ratio##keepRatioWorld", &constScaleRatio_w);
							ImGui::DragFloat("##X_WS", &(scale_w->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
							if (constScaleRatio_w)
							{
								scale_w->y = scale_w->z = scale_w->x;
							}
							else
							{
							ImGui::DragFloat("##Y_WS", &(scale_w->y),  0.01f, 0, 0, "%.1f"); ImGui::SameLine();
							ImGui::DragFloat("##Z_WS", &(scale_w->z),  0.01f, 0, 0, "%.1f"); ImGui::SameLine();
							}
							if (ImGui::Button("reset##WS"))
							{
								activeMesh->ResetUserTransform_scale_world();
							}

							// Delete model
							ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
							if (ImGui::Button("Delete model"))
							{
								models.erase(models.begin() + activeModel);
								activeModel = NOT_SELECTED;
							}
							ImGui::PopStyleColor(3);
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


	//---------------------------------------------------------
	//-------- Check if the popup should be shown -------------
	//---------------------------------------------------------
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
		ImGui::SetKeyboardFocusHere();
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
		ImGui::Button("OK");
		if ((ImGui::IsItemClicked() || ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Enter)))
			&& arePositionValuesValid && isNameValid)
		{
			GUI_popup_pressedOK = true;
			GUI_popup_pressedCANCEL = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		ImGui::Button("Cancel");
		if (ImGui::IsItemClicked() || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
		{
			GUI_popup_pressedOK = false;
			GUI_popup_pressedCANCEL = true;
			ImGui::CloseCurrentPopup();
			showTransWindow = false;
		}


		ImGui::EndPopup();
	}

	//------------------------------------
	//--------  Handle pop-ups -----------
	//------------------------------------
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
			currCamera->setStartPosition(vec4(posBuffer[0], posBuffer[1], posBuffer[2], 0));
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
	ImVec2 imgPos = ImVec2(viewportX, viewportY);
	ImVec2 imgSize = ImVec2(viewportWidth, viewportHeight);

	ImGui::SetNextWindowPos (imgPos);
	ImGui::SetNextWindowSize(imgSize);

	if (ImGui::Begin("Main Window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove			 | \
											ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | \
											ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMouseInputs))
	{
		ImGui::Image((void*)(intptr_t)(m_renderer->m_textureID), imgSize);
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

void Scene::setViewPort(vec4& vp)
{
	viewportX	   = vp.x;
	viewportY	   = vp.y;
	viewportWidth  = vp.z;
	viewportHeight = vp.w;
}

void Scene::resize_callback_handle(int width, int height)
{
	/* Take the main menu bar height into account */
	float mainMenuBarHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().FramePadding.y * 2.0f;
	height -= (int)mainMenuBarHeight;
	
	/* Take the Transformatins Widnow width into account (if opened) */
	int transormationWindowGap = showTransWindow ? transformationWindowWidth : 0;
	width -= transormationWindowGap;


	// Calculate aspect ratio
	float aspect = (float)width / (float)height;

	// Calculate new viewport size
	int newWidth = width, newHeight = height;
	if (aspect > modelAspectRatio)
		newWidth = (int)((float)height * modelAspectRatio);
	else
		newHeight = (int)((float)width / modelAspectRatio);

	// Calculate viewport position to keep it centered
	int xOffset = (abs(width - newWidth) / 2) + transormationWindowGap;
	int yOffset = (abs(height - newHeight) / 2) + mainMenuBarHeight;

	// Set viewport
	glViewport(xOffset, yOffset, newWidth, newHeight);

	//Update buffer
	m_renderer->update(newWidth, newHeight);

	//Update Scene
	setViewPort(vec4(xOffset, yOffset, newWidth, newHeight));
}