#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "stdafx.h"
#include <cstdlib>
#include "CG_skel_w_glfw.h"
#include "GL/glew.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>

using namespace std;

//----------------------------------------------------------------------------
// ---------------------- Constants defines  ---------------------------------
//----------------------------------------------------------------------------
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

//----------------------------------------------------------------------------
// ---------------------- Global variables  ----------------------------------
//----------------------------------------------------------------------------
Scene* scene;
Renderer* renderer;
bool lb_down, rb_down, mb_down; //mouse buttons (left/right/middle)
bool cam_mode;	// Camera mode ON/OFF
vec2 mouse_pos, mouse_pos_prev;
float mouse_scroll;


//----------------------------------------------------------------------------
// ---------------------- Callbacks functions --------------------------------
//----------------------------------------------------------------------------

// Error
void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// keyboard pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	// Let ImGui handle the callback first:
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

	//Our callback:
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
			case GLFW_KEY_F: //Set focus on active model
				if (scene->activeCamera != NOT_SELECTED && scene->activeModel != NOT_SELECTED)
				{
					scene->GetActiveCamera()->LookAt(scene->GetActiveModel());
				}
				break;
		}
	}
}

//mouse position
void mouse_move_callback(GLFWwindow* window, double x, double y)
{
	// Let ImGui handle the callback first:
	ImGui_ImplGlfw_CursorPosCallback(window, x, y);

	//Our callback:
	mouse_pos_prev = mouse_pos;
	mouse_pos = vec2(x, y);
}

//mouse click
void mouse_click_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Let ImGui handle the callback first:
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

	//Our callback:
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		lb_down = action == GLFW_PRESS;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		rb_down = action == GLFW_PRESS;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		mb_down = action == GLFW_PRESS;
		break;
	}
}

//mouse scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Let ImGui handle the callback first:
	ImGui_ImplGlfw_ScrollCallback(window, xoffset,yoffset);

	//Our callback:
	scene->zoom(yoffset);
}

//resize window
void resize_callback(GLFWwindow* window, int width, int height)
{
	if (!renderer || !scene)
		return;
	scene->resize_callback_handle(width, height);
}

int my_main(int argc, char** argv)
{
	
//----------------------------------------------------------------------------
// ---------------------- glfw init ------------------------------------------
//----------------------------------------------------------------------------
	string title = "CG";

	if (!glfwInit())
	{
		printf("GLFW: glfw init failed");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	GLFWwindow* window = glfwCreateWindow(START_WIDTH, START_HEIGHT, title.c_str(), NULL, NULL);
	if (!window)
	{
		// Window or OpenGL context creation failed
		fprintf(stderr, "Error: GLFW: window creation failed");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

//----------------------------------------------------------------------------
//------------------------------ glew stuff ----------------------------------
//----------------------------------------------------------------------------

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "glewInit Error: %s\n", glewGetErrorString(err));
		glfwTerminate();
		/*		...*/
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

//----------------------------------------------------------------------------
//------------------------------ ImGui stuff ---------------------------------
//----------------------------------------------------------------------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= (ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad);
	io.Fonts->AddFontFromFileTTF("../imgui/fonts/NotoSans-Medium.ttf", 18.0f);
	

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true); // install_callback=true installs GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init(NULL);

//----------------------------------------------------------------------------
//------------------------ Renderer + Scene init -----------------------------
//----------------------------------------------------------------------------
	renderer = new Renderer(START_WIDTH, START_HEIGHT, window);
	scene = new Scene(renderer);
	


//----------------------------------------------------------------------------
//--------------------------- Bind Callbacks ---------------------------------
//----------------------------------------------------------------------------
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetMouseButtonCallback(window, mouse_click_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, resize_callback);


//----------------------------------------------------------------------------
//------------------------------- Main Loop ----------------------------------
//----------------------------------------------------------------------------
	
	//Manally call for resize window to start up the window properly
	resize_callback(window, START_WIDTH, START_HEIGHT);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); //consider using glfwWaitEvents() ...

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		glUseProgram(renderer->program);
		scene->draw();
		scene->drawGUI();
		//ImGui::ShowDemoWindow();
		
		/* Render the scene */
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		renderer->SwapBuffers();
	}
	
//----------------------------------------------------------------------------
//------------------------------- Terminate ----------------------------------
//----------------------------------------------------------------------------
	//glDeleteTextures(1, &(renderer->m_textureID));
	glfwDestroyWindow(window);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	if(scene)
		delete scene;
	if(renderer)
		delete renderer;
	
	return 0;
}


//----------------------------------------------------------------------------
//---------------------- Main (don't touch) ----------------------------------
//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    int nRetCode = 0;
	glfwSetErrorCallback(error_callback);	// To track errors during & after init
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	//if (system("cmd /c start"))
	//{
	//	cout << "Fatal Error: CMD initialization failed" << endl;
	//	nRetCode = 1;
	//}
	else
	{
		nRetCode = my_main(argc, argv);
	}
	//nRetCode = my_main(argc, argv);

	glfwTerminate();
	return nRetCode;

}