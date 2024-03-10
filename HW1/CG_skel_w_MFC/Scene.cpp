#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "PrimMeshModel.h"
#include "CG_skel_w_glfw.h"
#include <string>
#include <math.h>

#define CLAMP(x, l, r) (min( max((x), (l)) , (r)))

#define MODEL_TAB_INDEX  0
#define CAMERA_TAB_INDEX 1
#define LIGHT_TAB_INDEX  2

using namespace std;

static char nameBuffer[64] = { 0 };
static float posBuffer[3] = { 0 };
static int g_ortho = 1;
static int light_type_radio_button;
static bool saved_palette_init = true;
static ImVec4 saved_palette[32] = {};

bool add_showModelDlg = false, add_showCamDlg = false, add_showLightDlg = false;
bool showTransWindow = false;
bool constScaleRatio = false;
bool constScaleRatio_w = false;
int transformationWindowWidth = 0;
bool closedTransfWindowFlag = false;


const float FOV_RANGE_MIN = -179, FOV_RANGE_MAX = 179;
const float ASPECT_RANGE_MIN = -10, ASPECT_RANGE_MAX = 10;
const float PROJ_RANGE_MIN = -20, PROJ_RANGE_MAX = 20;
const float ZCAM_RANGE_MIN = 0.001, ZCAM_RANGE_MAX = 100;
const float TRNSL_RANGE_MIN = -40, TRNSL_RANGE_MAX = 40;
const float ROT_RANGE_MIN = -180, ROT_RANGE_MAX = 180;
const float SCALE_RANGE_MIN = -1, SCALE_RANGE_MAX = 20;


//--------------------------------------------------
//-------------------- CAMERA ----------------------
//--------------------------------------------------
Camera::Camera()
{
	name = CAMERA_DEFAULT_NAME;
	
	iconInit();
	ResetRotation();
	ResetTranslation();
	resetProjection();
	setOrtho();
	LookAt();
	name = CAMERA_DEFAULT_NAME;
}

void Camera::iconInit()
{
	num_icon_vertices = 6;	// Or 4 - optional
	icon = new vec3[num_icon_vertices];
	iconBuffer = new vec2[num_icon_vertices];
	icon[0] = vec3(1, 0, 0);
	icon[1] = vec3(-1, 0, 0);
	icon[2] = vec3(0, 1, 0);
	icon[3] = vec3(0, -1, 0);
	icon[4] = vec3(0, 0, 0);	// Optional
	icon[5] = vec3(0, 0, -2);	// optional
}

bool Camera::iconDraw( mat4& active_cTransform, mat4& active_projection)
{
	for (int i = 0; i < num_icon_vertices; i++)
	{
		
		vec4 v_i(icon[i]);

		//Apply transformations:
		v_i = active_cTransform * (transform_mid_worldspace * v_i);

		//Project:
		v_i = active_projection * v_i;
		
		v_i /= v_i.w;

		//Clip it:
		if (v_i.x < -1 || v_i.x > 1 || v_i.y < -1 || v_i.y > 1 || v_i.z < -1 || v_i.z > 1)
		{
			return false;
		}

		//Add to buffer:
		iconBuffer[i] = vec2(v_i.x, v_i.y);
	}
	
	return true;
}

mat4 Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up)
{
	vec4 n, u, v;
	n = normalize(eye - at);
	
	//Edge case: Camera is directly above / below the target (up is parallel to n)
	if (vec3(n.x, n.y, n.z) == vec3(up.x, up.y, up.z) || vec3(n.x, n.y, n.z) == -vec3(up.x, up.y, up.z))
	{
		/* Add little noise so it won't divide by 0 ;) */
		n.x += 0.001f;
	}
	
	u = normalize(cross(up, n));
	v = normalize(cross(n, u));
	
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	n.w = u.w = v.w = 0;
	mat4 c = mat4(u, v, n, t);

	mat4 result = c * Translate(-eye);

	return result;

}

//This function is called from keyboard manager (pressed F to set focus)
void Camera::LookAt(const Model* target)
{
	/* Camera position */
	vec4 eye = c_trnsl;
 
	/* Target position */
	if (target)
		this->target = ((MeshModel*)target)->getCenterOffMass();
	else
		this->target = vec4(0, 0, 0, 1);

	/* Up vector */
	vec4 up = vec4(0, 1, 0, 1);

	/* Get LookAt matrix */
	mat4 lookAtMat = LookAt(eye, this->target, up);

	/* Get Euler angles by rotation matrix */
	const float m11 = lookAtMat[0][0], m12 = lookAtMat[0][1], m13 = lookAtMat[0][2];
	const float m21 = lookAtMat[1][0], m22 = lookAtMat[1][1], m23 = lookAtMat[1][2];
	const float m31 = lookAtMat[2][0], m32 = lookAtMat[2][1], m33 = lookAtMat[2][2];
	float x, y, z;
	y = asinf(CLAMP(m13, -1, 1));
	if (abs(m13) < 0.9999999)
	{
		x = atan2f(-m23, m33);
		z = atan2f(-m12, m11);
	}
	else
	{
		x = atan2f(m32, m22);
		z = 0;
	}

	/* Radians to Degrees */
	x *= 180 / M_PI;
	y *= 180 / M_PI;
	z *= 180 / M_PI;
	

	/* Update camera's vectors*/
	c_rot = vec4(-x, y, z, 1);
	c_rot_viewspace = vec4(0, 0, 0, 1);
	c_trnsl_viewspace = vec4(0, 0, 0, 1);
	updateTransform();
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
	
	c_bottom = -c_top;
	c_left = -c_right;

	
	setPerspective();
}

void Camera::setPerspectiveByParams()
{	
	if (!lockFov_GUI)
	{
		float width  = (c_right - c_left);
		float height = (c_top - c_bottom);
		c_aspect = c_right / c_top;
		c_fovy = (180 / M_PI) * atanf(c_top / c_zNear);
		setPerspective();
	}
	else
	{
		// Fovy is locked + user changed zNear
		// Calcualte new left right top bottom
		setPerspectiveByFov();
	}
	

}

void Camera::resetProjection()
{

	c_left = c_bottom = -DEF_PARAM;
	c_right = c_top = DEF_PARAM;
	c_zNear = DEF_ZNEAR;
	c_zFar = DEF_ZFAR;

	c_fovy = DEF_FOV;
	c_aspect = DEF_ASPECT;

	if (isOrtho)
		setOrtho();
	else
		setPerspectiveByFov();

	updateTransform();

}

void Camera::updateTransform()
{
	mat4 rot_x = RotateX(c_rot.x);
	mat4 rot_y = RotateY(c_rot.y);
	mat4 rot_z = RotateZ(c_rot.z);

	mat4 rot = transpose(rot_z * (rot_y * rot_x));
	mat4 trnsl = Translate(-c_trnsl);



	//Save mid results for camera icon view
	transform_mid_worldspace = Translate(c_trnsl) * transpose(rot);
	
	// C-t  = R^T * T^-1
	cTransform = rot * trnsl; // Mid result of cTransform


	//Apply view-space transformations:
	mat4 rot_x_view = RotateX(c_rot_viewspace.x);
	mat4 rot_y_view = RotateY(c_rot_viewspace.y);
	mat4 rot_z_view = RotateZ(c_rot_viewspace.z);

	mat4 rot_view = rot_z_view * (rot_y_view * rot_x_view);
	mat4 trnsl_view = Translate(c_trnsl_viewspace);
	
	cTransform = trnsl_view * (rot_view * cTransform);

	//Save mid results for camera icon view
	transform_mid_viewspace = trnsl_view * (rot_view * transform_mid_worldspace);

	//Save the inverse rotation matrix for normals display:
	rotationMat_normals = rot_view * rot;

}

void Camera::zoom(double s_offset, double update_rate)
{
	// Change projection according to the scroll offset of the user
	double offset_tot = s_offset * update_rate;
	c_top -= offset_tot;
	c_bottom += offset_tot;
	c_right -= offset_tot;
	c_left += offset_tot;
	lockFov_GUI = false; //Unlock the params


	if (isOrtho)
		setOrtho();
	else
		setPerspectiveByParams();
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

void Scene::AddLight()
{
	Light* lightSource = new Light();
	lights.push_back(lightSource);

	string s = lightSource->getName();
	s += " " + std::to_string(lights.size());
	lightSource->setName(s); //Light 1, Light 2, Light 3 ...
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
		if (pos != std::string::npos)
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
	add_showLightDlg = false;			// Reset flag
	memset(nameBuffer, 0, IM_ARRAYSIZE(nameBuffer));
	memset(posBuffer, 0, sizeof(float) * 3);
}

void Scene::draw()
{
	//1. GUI
	drawGUI();

	//2. Clear the pixel buffer before drawing new frame, reset y min/max pixel coordinates
	m_renderer->clearBuffer();

	//3. Update all light sources position in camera space (Multiply with the cTransform matrix...)
	for (auto light : lights)
	{
		light->updatePositionCameraSpace(cameras[activeCamera]->cTransform);
	}

	//4. draw each MeshModel
	for (auto model : models)
	{
		//Don't draw new model before user clicked 'OK'.
		if (!model->GetUserInitFinished())
			continue;

		model->draw(cameras[activeCamera]->cTransform, 
					cameras[activeCamera]->projection,
					cameras[activeCamera]->allowClipping,
					cameras[activeCamera]->rotationMat_normals);

		m_renderer->ResetMinMaxY();
		unsigned int len = 0;
		switch (draw_algo)	// Light calculation, material calculation
		{
			case WIRE_FRAME:
			{
				//values: [-1, 1]
				auto vertices = ((MeshModel*)model)->GetBuffer();
				len = ((MeshModel*)model)->GetBuffer_len(MODEL);
				if (vertices)
					m_renderer->Rasterize_WireFrame(vertices, len);
				break;
			}
			case FLAT:
			{
				m_renderer->Rasterize_Flat((const MeshModel*)model);
				break;
			}
			case GOURAUD:
			{

				break;
			}
			case PHONG:
			{

				break;
			}
		}


		// Bounding Box
		if (((MeshModel*)model)->showBoundingBox)
		{
			vec2* bbox_vertices = ((MeshModel*)model)->GetBuffer(BBOX);
			len = ((MeshModel*)model)->GetBuffer_len(BBOX);
			if (bbox_vertices)
				m_renderer->SetBufferLines(bbox_vertices, len, vec4(0, 1, 0, 1));
		}

		// Vertex Normals
		if (((MeshModel*)model)->showVertexNormals)
		{
			vec2* v_norm_vertices = ((MeshModel*)model)->GetBuffer(V_NORMAL);
			len = ((MeshModel*)model)->GetBuffer_len(V_NORMAL);
			if (v_norm_vertices)
				m_renderer->SetBufferLines(v_norm_vertices, len, vec4(1, 0, 0));

		}

		// Face normals
		if (((MeshModel*)model)->showFaceNormals)
		{
			vec2* f_norm_vertices = ((MeshModel*)model)->GetBuffer(F_NORMAL);
			len = ((MeshModel*)model)->GetBuffer_len(F_NORMAL);
			if (f_norm_vertices)
				m_renderer->SetBufferLines(f_norm_vertices, len, vec4(0, 0, 1));
		}


	}

	//5. Render cameras as 3D plus signs
	for (auto camera : cameras)
	{
		if (camera->renderCamera && camera != cameras[activeCamera])
		{
			if (camera->iconDraw(cameras[activeCamera]->cTransform, cameras[activeCamera]->projection))
			{
				vec2* icon_vertices = camera->getIconBuffer();
				unsigned int len = camera->getIconBufferSize();
				if (icon_vertices)
				{
					m_renderer->SetBufferLines(icon_vertices, len-2, vec4(0.75, 0, 0.8));
					m_renderer->SetBufferLines((icon_vertices + len - 2), 2, vec4(0, 0.8, 0.15));
				}
			}
		}
	}

	//6. Update the texture. (OpenGL stuff)
	m_renderer->updateTexture();
  
}

void colorPicker(ImVec4* color, std::string button_label, std::string id)
{
	ImGuiColorEditFlags flags = (ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_InputRGB);

	if (saved_palette_init)
	{
		for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
		{
			ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
				saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
			saved_palette[n].w = 1.0f; // Alpha
		}
		saved_palette_init = false;
	}

	bool open_popup = ImGui::ColorButton((button_label+id).c_str(), *color, flags);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text(button_label.c_str());
	if (open_popup)
		ImGui::OpenPopup(("Color Palette"  + id).c_str());

	if (ImGui::BeginPopup(("Color Palette" + id).c_str()))
	{
		ImGui::ColorPicker4(id.c_str(), (float*)color, flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoAlpha);

		ImGui::EndPopup();
	}
}

void Scene::drawCameraTab()
{
	string name(cameras[activeCamera]->getName());
	ImGui::SeparatorText(name.c_str());

	ImGui::Checkbox("Render Camera", &cameras[activeCamera]->renderCamera); ImGui::SameLine();
	bool* g_allowClipping = &(cameras[activeCamera]->allowClipping);
	ImGui::Checkbox("Allow clipping", g_allowClipping);


	float* g_left = &(cameras[activeCamera]->c_left);
	float* g_right = &(cameras[activeCamera]->c_right);
	float* g_top = &(cameras[activeCamera]->c_top);
	float* g_bottom = &(cameras[activeCamera]->c_bottom);
	float* g_zNear = &(cameras[activeCamera]->c_zNear);
	float* g_zFar = &(cameras[activeCamera]->c_zFar);
	float* g_fovy = &(cameras[activeCamera]->c_fovy);
	float* g_aspect = &(cameras[activeCamera]->c_aspect);
	vec4* g_rot = &(cameras[activeCamera]->c_rot);
	vec4* g_trnsl = &(cameras[activeCamera]->c_trnsl);

	vec4* g_rot_view   = &(cameras[activeCamera]->c_rot_viewspace);
	vec4* g_trnsl_view = &(cameras[activeCamera]->c_trnsl_viewspace);


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


	if (g_ortho == 1) /* Ortho type */
	{
		if (cameras[activeCamera]->isOrtho == false)
		{
			cameras[activeCamera]->isOrtho = true;
			cameras[activeCamera]->resetProjection();
		}

		if (prev_left != *g_left || prev_right != *g_right ||
			prev_bottom != *g_bottom || prev_top != *g_top ||
			prev_zNear != *g_zNear || prev_zFar != *g_zFar)
		{
			cameras[activeCamera]->setOrtho();
		}
	}
	else /* Perspective type */
	{
		if (cameras[activeCamera]->isOrtho == true)
		{
			cameras[activeCamera]->isOrtho = false;
			cameras[activeCamera]->resetProjection();
		}
		
		float prev_fovy = *g_fovy;
		float prev_aspect = *g_aspect;

		ImGui::DragFloat("##FovY", g_fovy, 0.01f, FOV_RANGE_MIN, FOV_RANGE_MAX, "FovY = %.1f "); ImGui::SameLine();
		ImGui::DragFloat("##Aspect", g_aspect, 0.01f, ASPECT_RANGE_MIN, ASPECT_RANGE_MAX, "Aspect = %.1f "); ImGui::SameLine();
		ImGui::Checkbox("Lock FovY", cameras[activeCamera]->getLockFovyPTR());

		if (prev_fovy != *g_fovy || prev_aspect != *g_aspect) //User changed FOV or Aspect Ratio
		{
			cameras[activeCamera]->setPerspectiveByFov();
		}
		else if( prev_left != *g_left || prev_right != *g_right || prev_bottom != *g_bottom ||
				 prev_top  != *g_top  || prev_zNear != *g_zNear || prev_zFar != *g_zFar )
		{
			if (prev_zNear == *g_zNear)
			{
				//zNear didn't changed - other paramater changed - UnLock fovy.
				cameras[activeCamera]->unLockFovy();
			}
			
			cameras[activeCamera]->setPerspectiveByParams();
		}

	}
	
	if (ImGui::Button("reset"))
	{
		cameras[activeCamera]->resetProjection();
	}


	ImGui::SeparatorText("View space");

	vec4 prev_trnsl_view = *g_trnsl_view;
	vec4 prev_rot_view = *g_rot_view;

	ImGui::Text("Translation (X Y Z)");
	ImGui::DragFloat("##X_VT", &(g_trnsl_view->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
	ImGui::DragFloat("##Y_VT", &(g_trnsl_view->y), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
	ImGui::DragFloat("##Z_VT", &(g_trnsl_view->z), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
	if (ImGui::Button("reset##VT"))
	{
		cameras[activeCamera]->ResetTranslation_viewspace();
	}

	ImGui::Text("Rotation (X Y Z)");
	ImGui::DragFloat("##X_VR", &(g_rot_view->x), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
	ImGui::DragFloat("##Y_VR", &(g_rot_view->y), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
	ImGui::DragFloat("##Z_VR", &(g_rot_view->z), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
	if (ImGui::Button("reset##VR"))
	{
		cameras[activeCamera]->ResetRotation_viewspace();
	}

	if (prev_trnsl_view != *g_trnsl_view || prev_rot_view != *g_rot_view)
	{
		cameras[activeCamera]->updateTransform();
	}


	ImGui::SeparatorText("World space");

	vec4 prev_trnsl = *g_trnsl;
	vec4 prev_rot = *g_rot;

	ImGui::Text("Translation (X Y Z)");
	ImGui::DragFloat("##X_MT", &(g_trnsl->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
	ImGui::DragFloat("##Y_MT", &(g_trnsl->y), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
	ImGui::DragFloat("##Z_MT", &(g_trnsl->z), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
	if (ImGui::Button("reset##MT"))
	{
		cameras[activeCamera]->ResetTranslation();
	}

	ImGui::Text("Rotation (X Y Z)");
	ImGui::DragFloat("##X_MR", &(g_rot->x), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
	ImGui::DragFloat("##Y_MR", &(g_rot->y), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
	ImGui::DragFloat("##Z_MR", &(g_rot->z), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
	if (ImGui::Button("reset##MR"))
	{
		cameras[activeCamera]->ResetRotation();
	}

	if (prev_trnsl != *g_trnsl || prev_rot != *g_rot)
	{
		cameras[activeCamera]->updateTransform();
	}

	if (cameras.size() > 1)
	{
		// Delete camera
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
		if (ImGui::Button("Delete camera"))
		{
			cameras.erase(cameras.begin() + activeCamera);
			activeCamera = max(0, activeCamera - 1);
		}
		ImGui::PopStyleColor(3);
	}
}

void Scene::drawModelTab()
{
	MeshModel* activeMesh = (MeshModel*)models[activeModel];
	string name(models[activeModel]->getName());
	ImGui::SeparatorText(name.c_str());

	vec4* g_trnsl = &(activeMesh->_trnsl);
	vec4* g_rot = &(activeMesh->_rot);

	bool* dispFaceNormal = &(activeMesh->showFaceNormals);
	bool* dispVertexNormal = &(activeMesh->showVertexNormals);
	bool* dispBoundingBox = &(activeMesh->showBoundingBox);

	float* lenFaceNormal = activeMesh->getLengthFaceNormal();
	float* lenVertNormal = activeMesh->getLengthVertexNormal();

	ImGui::Checkbox("Display Face Normals  ", dispFaceNormal);
	
	if (*dispFaceNormal)
	{
		ImGui::SameLine();
		ImGui::DragFloat("Length##Length_Face_normal", lenFaceNormal, 0.001f, 0, 0, "%.3f");
	}

	ImGui::Checkbox("Display Vertex Normals", dispVertexNormal);
	if (*dispVertexNormal)
	{
		ImGui::SameLine();
		ImGui::DragFloat("Length##Length_Vert_normal", lenVertNormal, 0.001f, 0, 0, "%.3f");
	}

	ImGui::Checkbox("Display Bounding Box", dispBoundingBox);

	// Model material
	if (ImGui::CollapsingHeader("Model Material"))
	{
		Material* meshMaterial = activeMesh->getMaterial();
		vec3& emis_real = meshMaterial->getEmissive();
		vec3& diff_real = meshMaterial->getDiffuse();
		vec3& spec_real = meshMaterial->getSpecular();

		ImVec4 emis_local = ImVec4(emis_real.x, emis_real.y, emis_real.z, 1);
		ImVec4 diff_local = ImVec4(diff_real.x, diff_real.y, diff_real.z, 1);
		ImVec4 spec_local = ImVec4(spec_real.x, spec_real.y, spec_real.z, 1);

		colorPicker(&emis_local, "Emissive Color", "##pickerEmis");
		colorPicker(&diff_local, "Diffuse Color", "##pickerDiff");
		colorPicker(&spec_local, "Specular Color", "##pickerSpec");

		emis_real.x = emis_local.x;
		emis_real.y = emis_local.y;
		emis_real.z = emis_local.z;

		diff_real.x = diff_local.x;
		diff_real.y = diff_local.y;
		diff_real.z = diff_local.z;

		spec_real.x = spec_local.x;
		spec_real.y = spec_local.y;
		spec_real.z = spec_local.z;

		float* ka = &(meshMaterial->Ka);
		float* kd = &(meshMaterial->Kd);
		float* ks = &(meshMaterial->Ks);
		float* emissivefactor = &(meshMaterial->EmissiveFactor);
		int* alphaFactor = &(meshMaterial->COS_ALPHA);

		ImGui::SeparatorText("Intensity");
		ImGui::Text("Ambient Intensity (Ka)"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2, 0);
		ImGui::DragFloat("##K_amb", ka, 0.001f, 0, 10, "%.3f");

		ImGui::Text("Diffuse Intensity (Kd)"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2, 0);
		ImGui::DragFloat("##K_dif", kd, 0.001f, 0, 10, "%.3f");

		ImGui::Text("Specular Intensity (Ks)"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2, 0);
		ImGui::DragFloat("##K_spc", ks, 0.001f, 0, 10, "%.3f");

		ImGui::Text("Emissive factor"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2, 0);
		ImGui::DragFloat("##K_emsv", emissivefactor, 0.001f, 0, 1, "%.3f");

		ImGui::Text("ALPHA factor"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2, 0);
		ImGui::DragInt("##K_alpha", alphaFactor, 0.01f, 0, 5);
		if (ImGui::Button("Reset all##RK"))
		{
			*ka = 0.5f;
			*kd = 0.5f;
			*ks = 0.5f;
			*emissivefactor = 0.5f;
			*alphaFactor = 1;
		}
	}

	// Model position
	if (ImGui::CollapsingHeader("Model Position"))
	{

		ImGui::SeparatorText("Model space");


		ImGui::Text("Translation (X Y Z)");
		ImGui::DragFloat("##X_MT", &(g_trnsl->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
		ImGui::DragFloat("##Y_MT", &(g_trnsl->y), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
		ImGui::DragFloat("##Z_MT", &(g_trnsl->z), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
		if (ImGui::Button("reset##MT"))
		{
			activeMesh->ResetUserTransform_translate_model();
		}

		ImGui::Text("Rotation (X Y Z)");
		ImGui::DragFloat("##X_MR", &(g_rot->x), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
		ImGui::DragFloat("##Y_MR", &(g_rot->y), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
		ImGui::DragFloat("##Z_MR", &(g_rot->z), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
		if (ImGui::Button("reset##MR"))
		{
			activeMesh->ResetUserTransform_rotate_model();
		}

		vec4* g_scale = &(activeMesh->_scale);

		ImGui::Text("Scale (X Y Z)");
		ImGui::Checkbox("keep ratio", &constScaleRatio);
		ImGui::DragFloat("##X_MS", &(g_scale->x), 0.01f, 0, 0, "%.3f"); ImGui::SameLine();
		if (constScaleRatio)
		{
			g_scale->y = g_scale->z = g_scale->x;
		}
		else
		{
			ImGui::DragFloat("##Y_MS", &(g_scale->y), 0.01f, 0, 0, "%.3f"); ImGui::SameLine();
			ImGui::DragFloat("##Z_MS", &(g_scale->z), 0.01f, 0, 0, "%.3f"); ImGui::SameLine();
		}

		if (ImGui::Button("reset##MS"))
		{
			activeMesh->ResetUserTransform_scale_model();
		}


		// World transformations
		ImGui::SeparatorText("World space");

		vec4* trnsl_w = &(activeMesh->_trnsl_w);
		vec4* rot_w = &(activeMesh->_rot_w);
		vec4* scale_w = &(activeMesh->_scale_w);


		ImGui::Text("Translation (X Y Z)");
		ImGui::DragFloat("##X_WT", &(trnsl_w->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
		ImGui::DragFloat("##Y_WT", &(trnsl_w->y), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
		ImGui::DragFloat("##Z_WT", &(trnsl_w->z), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();

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
		ImGui::DragFloat("##X_WS", &(scale_w->x), 0.01f, 0, 0, "%.3f"); ImGui::SameLine();
		if (constScaleRatio_w)
		{
			scale_w->y = scale_w->z = scale_w->x;
		}
		else
		{
			ImGui::DragFloat("##Y_WS", &(scale_w->y), 0.01f, 0, 0, "%.3f"); ImGui::SameLine();
			ImGui::DragFloat("##Z_WS", &(scale_w->z), 0.01f, 0, 0, "%.3f"); ImGui::SameLine();
		}
		if (ImGui::Button("reset##WS"))
		{
			activeMesh->ResetUserTransform_scale_world();
		}

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

void Scene::drawLightTab()
{
	Light* currentLight = (Light*)lights[activeLight];
	string name(currentLight->getName());
	ImGui::SeparatorText(name.c_str());

	vec3* light_pos = currentLight->getPositionPtr();
	vec3* light_dir = currentLight->getDirectionPtr();
	light_type_radio_button = (int)currentLight->getLightType();

	ImGui::RadioButton("Ambient",  &light_type_radio_button, AMBIENT_LIGHT); //ImGui::SameLine();
	ImGui::RadioButton("Point",    &light_type_radio_button, POINT_LIGHT);   //ImGui::SameLine();
	ImGui::RadioButton("Parallel", &light_type_radio_button, PARALLEL_LIGHT);

	currentLight->setLightType(light_type_radio_button);


	vec3& color = currentLight->getColor();

	ImVec4 color_local = ImVec4(color.x, color.y, color.z, 1);

	colorPicker(&color_local, "Color", "##LightColor");

	color.x = color_local.x;
	color.y = color_local.y;
	color.z = color_local.z;
	
	if (currentLight->getLightType() == POINT_LIGHT)
	{
		ImGui::SeparatorText("Position");

		vec3* pos = currentLight->getPositionPtr();

		ImGui::Text("Position (X Y Z)");
		ImGui::DragFloat("##X_WPL", &(pos->x), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
		ImGui::DragFloat("##Y_WPL", &(pos->y), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
		ImGui::DragFloat("##Z_WPL", &(pos->z), 0.01f, 0, 0, "%.1f"); ImGui::SameLine();
		if (ImGui::Button("reset##LP"))
		{
			currentLight->resetPosition();
		}
	}

	if (currentLight->getLightType() == PARALLEL_LIGHT)
	{
		ImGui::SeparatorText("Direction");

		vec3* dir = currentLight->getDirectionPtr();

		ImGui::Text("Direction (X Y Z)");
		ImGui::DragFloat("##X_WDL", &(dir->x), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
		ImGui::DragFloat("##Y_WDL", &(dir->y), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
		ImGui::DragFloat("##Z_WDL", &(dir->z), 0.1f, 0, 0, "%.0f"); ImGui::SameLine();
		if (ImGui::Button("reset##LD"))
		{
			currentLight->resetDirection();
		}
	}

	ImGui::SeparatorText("Light Intensity");

	float* la = &(currentLight->La);
	float* ld = &(currentLight->Ld);
	float* ls = &(currentLight->Ls);

	if (currentLight->getLightType() == AMBIENT_LIGHT)
	{
		ImGui::Text("Ambient Intensity "); ImGui::SameLine();
		ImGui::DragFloat("##I_amb", la, 0.001f, 0, 1, "%.3f");
	}

	else
	{
		ImGui::Text("Diffuse Intensity   "); ImGui::SameLine();
		ImGui::DragFloat("##I_dif", ld, 0.001f, 0, 1, "%.3f");
	
		ImGui::Text("Specular Intensity"); ImGui::SameLine();
		ImGui::DragFloat("##I_spc", ls, 0.001f, 0, 1, "%.3f");
	}


	if (ImGui::Button("Reset all intensity##RI"))
	{
		*la = 0.5f;
		*ld = 0.5f;
		*ls = 0.5f;
	}
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
	
	//UnSelect the object if it's transformation windows closed
	if (!showTransWindow && activeModel != NOT_SELECTED) 
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

				if (ImGui::MenuItem("Pyramid (square)"))
				{
					Pyramid* pyr = new Pyramid();
					models.push_back(pyr);

					strcpy(nameBuffer, pyr->getName().c_str());
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

			if (ImGui::MenuItem("Light Source"))
			{
				AddLight();
				strcpy(nameBuffer, lights.back()->getName().c_str());
				vec3 c_trnsl = lights.back()->getPosition();
				posBuffer[0] = c_trnsl.x;
				posBuffer[1] = c_trnsl.y;
				posBuffer[2] = c_trnsl.z;

				add_showLightDlg = true;
				showTransWindow  = true;
			}
			
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Select"))
		{
			if (models.size() > 0)
			{
				if (ImGui::BeginMenu("Models"))
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
			if (ImGui::BeginMenu("Cameras"))
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
			if (ImGui::BeginMenu("Lights"))
			{
				int len = lights.size();
				for (int c = 0; c < len; c++)
				{
					if (ImGui::MenuItem(lights[c]->getName().c_str(), NULL, &lights[c]->selected))
					{
						/* Deselect all others */
						for (int t = 0; t < len; t++)
							lights[t]->selected = false;

						/* Select current light */
						activeLight			= c;
						lights[c]->selected = true;
						showTransWindow		= true;
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu(); //End select
		}
		
		// Delete Model/Camera/Lights
		if (models.size() > 0 || cameras.size() > 1 || lights.size() > 1)
		{
			if (ImGui::BeginMenu("Delete..."))
			{
				if (models.size()  > 0)
				{
					if (ImGui::BeginMenu("Models"))
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
					if (ImGui::BeginMenu("Cameras"))
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
				if (lights.size()  > 1)	// Delete only if there is more than one light
				{
					if (ImGui::BeginMenu("Lights"))
					{
						for (int c = 0; c < lights.size(); c++)
						{
							if (ImGui::MenuItem(lights[c]->getName().c_str(), NULL))
							{
								/* Delete current camera */
								lights.erase(lights.begin() + c);

								if (c == activeLight)
								{
									activeLight = max(0, activeLight - 1);
								}
								else if (activeLight > c)
								{
									--activeLight;	// index changed
								}
								lights[activeLight]->selected = true;

							}
						}
						ImGui::EndMenu();
					}

				}

				ImGui::EndMenu(); //End delete
			}
		}
		if (ImGui::BeginMenu("Options"))
		{
			ImGui::SeparatorText("General");
			if(ImGui::BeginMenu("Shading Mode..."))
			{
				for (int i = 0; i < static_cast<int>(DrawAlgo::COUNT); ++i) {
					DrawAlgo algo = static_cast<DrawAlgo>(i);
					if (ImGui::MenuItem(drawAlgoToString(algo), NULL, this->draw_algo == algo))
						this->draw_algo = algo;
				}
				ImGui::EndMenu();
			}
			ImGui::SeparatorText("Cameras");
			if (ImGui::MenuItem("Render all cameras"))
			{
				for (auto camera : cameras)
					camera->renderCamera=true;
			}
			if (ImGui::MenuItem("Un-Render all cameras"))
			{
				for (auto camera : cameras)
					camera->renderCamera = false;
			}
			if (ImGui::MenuItem("Allow clipping"))
			{
				for (auto camera : cameras)
					camera->allowClipping = true;
			}
			if (ImGui::MenuItem("Disable clippping"))
			{
				for (auto camera : cameras)
					camera->allowClipping = false;
			}

			ImGui::EndMenu();	// End Options menu
		}

		ImGui::EndMainMenuBar();
	}


	//---------------------------------------------------------
	//------------ Transformations Window ---------------------
	//---------------------------------------------------------
	if (activeCamera != NOT_SELECTED && !add_showModelDlg && !add_showCamDlg && !add_showLightDlg && showTransWindow)
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

			const char* names[3] = { 0 };
			names[MODEL_TAB_INDEX]  = "Model";
			names[CAMERA_TAB_INDEX] = "Camera";
			names[LIGHT_TAB_INDEX]  = "Light";

			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("TransBar", tab_bar_flags))
			{
				ImGui::PushItemWidth(80);
				for (int n = 0; n < ARRAYSIZE(names); n++)
				{
					if (activeModel == NOT_SELECTED && n == MODEL_TAB_INDEX)
						continue;

					if (ImGui::BeginTabItem(names[n], 0, tab_bar_flags))
					{
						if (n == MODEL_TAB_INDEX)
						{
							drawModelTab();
						}
						else if (n == CAMERA_TAB_INDEX)
						{
							drawCameraTab();
						}
						else if (n == LIGHT_TAB_INDEX)
						{
							drawLightTab();
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
	if (add_showModelDlg || add_showCamDlg || add_showLightDlg)
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
	else if (add_showCamDlg)
	{
		if (GUI_popup_pressedOK)
		{
			auto currCamera = cameras.back();
			currCamera->setName(nameBuffer);
			
			
			currCamera->setStartPosition(vec4(posBuffer[0], posBuffer[1], posBuffer[2], 0));
			currCamera->updateTransform();
			ResetPopUpFlags();
		}
		else if (GUI_popup_pressedCANCEL)
		{
			delete cameras[cameras.size() - 1];
			cameras.pop_back();

			ResetPopUpFlags();
		}
	}
	else if (add_showLightDlg)
	{
		if (GUI_popup_pressedOK)
		{
			auto currLight = lights.back();
			currLight->setName(nameBuffer);			
			currLight->setPosition(vec3(posBuffer[0], posBuffer[1], posBuffer[2]));
			ResetPopUpFlags();
		}
		else if (GUI_popup_pressedCANCEL)
		{
			delete lights[lights.size() - 1];
			lights.pop_back();

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

Light* Scene::GetActiveLight()
{
	return (activeLight == NOT_SELECTED ? nullptr : lights[activeLight]);
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