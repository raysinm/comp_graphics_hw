// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define FILE_OPEN 1
#define MAIN_DEMO 1
#define MAIN_ABOUT 2
#define DebugButton 2

Scene *scene;
Renderer *renderer;

int last_x,last_y;
bool lb_down,rb_down,mb_down;

//----------------------------------------------------------------------------
// Callbacks

void display( void )
{
//Call the scene and ask it to draw itself
}

void reshape( int width, int height )
{
//update the renderer's buffers
}

void keyboard( unsigned char key, int x, int y )
{
#ifdef _DEBUG
	printf("keyboard called! key = %c   mouse (x,y) = (%d, %d)\n", key, x, y);
#endif
	switch ( key ) {
	case 033: /* ESC key*/
		exit( EXIT_SUCCESS );
		break;
#ifdef _DEBUG
	case 'a': /* Debug only !!! */
		debug_PlayWithVectors();
		break;
	case 'b':
		debug_PlayWithMatrices();
		break;
#endif
	}

}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}
	
	//set down flags
	switch(button) {
		case GLUT_LEFT_BUTTON:
			lb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_RIGHT_BUTTON:
			rb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_MIDDLE_BUTTON:
			mb_down = (state==GLUT_UP)?0:1;	
			break;
	}

	// add your code
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;
	// update last x,y
	last_x=x;
	last_y=y;
}

void fileMenu(int id)
{
	switch (id)
	{
		case FILE_OPEN:
			CFileDialog dlg(TRUE,_T(".obj"),NULL,NULL,_T("*.obj|*.*"));
			if(dlg.DoModal()==IDOK)
			{
				std::string s((LPCTSTR)dlg.GetPathName());
				scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
			}
			break;
	}
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_DEMO:
		scene->drawDemo();
		break;
	case MAIN_ABOUT:
		AfxMessageBox(_T("Computer Graphics"));
		break;
	}
}

void initMenu()
{

	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Open..",FILE_OPEN);
	glutCreateMenu(mainMenu);
	glutAddSubMenu("File",menuFile);
	glutAddMenuEntry("Demo",MAIN_DEMO);
	glutAddMenuEntry("About",MAIN_ABOUT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------



int my_main( int argc, char **argv )
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA| GLUT_DOUBLE);
	glutInitWindowSize( 512, 512 );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
#ifdef _DEBUG
	glutCreateWindow( "CG - DEBUG STATE !!!" );
#else
	glutCreateWindow( "CG" );
#endif
	glewExperimental = GL_TRUE;
	glewInit();
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		/*		...*/
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	
	
	//renderer = new Renderer(512,512);
	//scene = new Scene(renderer);
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	initMenu();
	

	glutMainLoop();
	//delete scene;
	//delete renderer;
	return 0;
}

CWinApp theApp;

using namespace std;

int main( int argc, char **argv )
{
	int nRetCode = 0;
	
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv );
	}
	
	return nRetCode;
}

#ifdef _DEBUG
void debug_PlayWithVectors()
{
	vec2 a = vec2(1);
	vec2 b = vec2(3, 4);
	cout << "debug_PlayWithVectors started" << endl;

	cout << "2d vector a = " << a << endl;
	cout << "2d vector b = " << b << endl;
	cout << "2d vector a+b = " << a+b << endl;
	a += b;
	cout << "a+=b. a = " << a << endl;
	cout << "a dot b = " << dot(a, b) << endl;
	cout << "length(b) = " << length(b) << endl;
	cout << "normalized b = " << normalize(b) << endl;

	cout << "debug_PlayWithVectors finished" << endl;
}

void debug_PlayWithMatrices()
{
	// mat2
	mat2 m2;
	mat3 m3;
	mat4 m4;
	cout << "debug_PlayWithVectors started" << endl;
	cout << "Initial mat2: " << m2;
	cout << "Initial mat3: " << m3;
	cout << "Initial mat4: " << m4;
	
	cout << "Test 1.1: mat2 operator*" << endl;
	mat2 m2a = mat2(1, 2, 1, 2);
	mat2 m2b = mat2(1, 1, 1, 1);
	cout << "m2a: " << m2a << endl;
	cout << "m2b: " << m2b << endl;
	cout << "result m2a * m2b: " << m2a * m2b << endl;
	cout << "result m2b * m2a: " << m2b * m2a << endl;
	
	cout << "Test 1.2: mat2 operator*=" << endl;
	m2b *= m2b;
	cout << "result m2b*=m2b: " << m2b << endl;

	cout << "Test 2.1: mat3 m*v, where m=I" << endl;
	vec3 v3 = vec3(1, 2, 3);
	cout << m3 * v3 << endl;
	
	cout << "Test 2.2: mat3 m*v, where m!=I" << endl;
	vec3 v3a = vec3(1, 1, 1);
	m3 = mat3(v3a, v3a, v3a);
	cout << m3 * v3 << endl;

	cout << "Test 3: row/col/transpose" << endl;
	mat2 m2c = mat2(1, 2, 3, 4);
	cout << "m2c: " << m2c << endl;
	cout << "m2c transposed: " << transpose(m2c) << endl;

	mat3 m3a = mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
	cout << "m3a: " << m3a << endl;
	cout << "m3a transposed: " << transpose(m3a) << endl;


	cout << "Test 4: transformations" << endl;
	mat4 scale = Scale(2, 2, 2);
	cout << "Scale mat: " << scale << endl;
	vec4 v4 = vec4(1, 1, 1, 1);
	cout << "Scaling by (2,2,2): " << Scale(2,2,2) * v4 << endl;
	cout << "Scaling by (1,2,3): " << Scale(1, 2, 3) * v4 << endl;

	mat4 rotate = RotateX(45);
	cout << "Rotate mat: " << rotate << endl;
	cout << "Rotating by 45: " << RotateX(45) * v4 << endl;
	cout << "Rotating by 360: " << RotateX(360) * v4 << endl;
	cout << "Rotating by 0: " << RotateX(0) * v4 << endl;
	cout << "Rotating by 180: " << RotateX(180) * v4 << endl;	// BUG? M: result is (1,-1,-1) but not (-1,-1,-1)

	mat4 trsl = Translate(2, 2, 2);
	cout << "Translate mat: " << trsl << endl;
	cout << "Translating by (2,2,2): " << Translate(2, 2, 2) * v4 << endl;
	cout << "Translating by (0,0,0): " << Translate(0,0,0) * v4 << endl;
	cout << "Translating by (-0.5,-0.5,0): " << Translate(-0.5, -0.5, 0) * v4 << endl;

	
	cout << "debug_PlayWithMatrices finished" << endl;
}
#endif