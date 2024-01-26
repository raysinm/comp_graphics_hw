//CG_skel_w_glfw.h

#include "stdafx.h"
#include "CG_skel_w_glfw.h"

//#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "GL/glew.h"

#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include "Button.h"

#include <string>

using namespace std;

//----------------------------------------------------------------------------
// ---------------------- Global variables  ---------------------------------
//----------------------------------------------------------------------------
std::vector<Button> buttons;
Scene* scene;
Renderer* renderer;
int last_x, last_y; // mouse positions
bool lb_down, rb_down, mb_down; //mouse buttons (left/right/middle)
bool cam_mode;	// Camera mode ON/OFF

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


int my_main(int argc, char** argv)
{
	
	//----------------------------------------------------------------------------
	// Initialize window
	if (!glfwInit())
	{
		printf("GLFW: glfw init failed");
		return 1;
	}
	string title = "CG";
#ifdef _DEBUG
	title += " - DEBUG!";
#endif/*
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

	GLFWwindow* window = glfwCreateWindow(640, 480, title.c_str(), NULL, NULL);
	if (!window)
	{
		// Window or OpenGL context creation failed
		fprintf(stderr, "Error: GLFW: window creation failed");
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// glew stuff
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

	renderer = new Renderer(512, 512, window);
	scene = new Scene(renderer);

	//----------------------------------------------------------------------------
	// Initialize Callbacks
	
	//----------------------------------------------------------------------------
	// Loop
	while (!glfwWindowShouldClose(window))
	{
		//glfwWaitEvenets();	maybe?
	}
	
	glfwDestroyWindow(window);	// On termination

	delete scene;
	delete renderer;
	return 0;
}




int main(int argc, char** argv)
{
    int nRetCode = 0;

	glfwSetErrorCallback(error_callback);	// To track errors during & after init

	//if (!glfwInit())
	//{
	//	_tprintf(_T("Fatal Error: MFC initialization failed\n"));
	//	nRetCode = 1;
	//}
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv);
	}

	glfwTerminate();

	return nRetCode;

}