#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "stdafx.h"
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
// ---------------------- Global variables  ----------------------------------
//----------------------------------------------------------------------------
Scene* scene;
Renderer* renderer;
bool lb_down, rb_down, mb_down; //mouse buttons (left/right/middle)
bool cam_mode;	// Camera mode ON/OFF
vec2 mouse_pos, mouse_pos_prev;
float mouse_scroll;

//----------------------------------------------------------------------------
// ---------------------- Constants defines  ---------------------------------
//----------------------------------------------------------------------------
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

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
			case GLFW_KEY_A:
#ifdef _DEBUG
				debug_func();
#endif
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
	mouse_scroll = yoffset;
}


#ifdef _DEBUG
void debug_func()
{
	cout << "SUCCESS - Pressed A" << endl; 
}
#endif // DEBUG


int my_main(int argc, char** argv)
{
	
//----------------------------------------------------------------------------
// ---------------------- glfw init ------------------------------------------
//----------------------------------------------------------------------------
	string title = "CG";
#ifdef _DEBUG
	title += " - DEBUG!";
#endif

	if (!glfwInit())
	{
		printf("GLFW: glfw init failed");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
	// Styling
	//ImGuiStyle& style = ImGui::GetStyle();
	//style.FramePadding.y = 15.0f;  // Adjust this value to change the main menu bar height

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


//----------------------------------------------------------------------------
//------------------------------- Main Loop ----------------------------------
//----------------------------------------------------------------------------
	
	bool imgui_show_demo = true;
	bool show_another_window = true;

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clear color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents(); //consider using glfwWaitEvents() ...

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();


		/* All the GUI and scene draw will happen from scene->draw fuction*/
		scene->draw();



		ImGui::ShowDemoWindow(&imgui_show_demo); // Show demo window! :)
/*
//#ifdef _DEBUG

		// 1. Show the big demo window
		/*
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &imgui_show_demo);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}
		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}
		*/

		/* Render the scene */
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/* Swap buffers */
		renderer->SwapBuffers();
	}
	
//----------------------------------------------------------------------------
//------------------------------- Terminate ----------------------------------
//----------------------------------------------------------------------------
	
	glDeleteTextures(1, &(renderer->m_textureID));

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
	else
	{
		nRetCode = my_main(argc, argv);
	}

	glfwTerminate();
	return nRetCode;

}