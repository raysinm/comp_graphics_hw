#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"

#include <string>

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

// Menu entry types
enum MenuEntry {
	MAIN_DEMO,
	MAIN_ABOUT,
	FILE_OPEN,
	SELECT_OBJECT,
	SELECT_CAMERA
};



//----------------------------------------------------------------------------
// ---------------------- Callbacks functions --------------------------------
//----------------------------------------------------------------------------
void btn_b1_clicked()
{
	cout << "B1 Clicked!" << endl;
}

void display( void )
{
//Call the scene and ask it to draw itself
	//cout << "display called" << endl;
	scene->draw();
}

void reshape( int width, int height )
{
//update the renderer's buffers
}

void keyboard( unsigned char key, int x, int y )
{
/* (x, y) is the mouse position in screen-space. TOP-LEFT corner is (0,0) */
#ifdef _DEBUG
	printf("keyboard called! key = %c   mouse (x, y) = (%d, %d)\n", key, x, y);
#endif
	switch ( key ) {
	case 033: /* ESC key*/
		exit( EXIT_SUCCESS );
		break;
#ifdef _DEBUG
	case 'a':
		debug_PlayWithVectors();
		break;
	case 'b':
		debug_PlayWithMatrices();
		break;
#endif
	case 'r':	// Rotation
		break;
	case 't':	// Translate
		break;
	case 's':	// Scale
		break;
	case 'c':	// Camera mode
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}
	
	vec2 mPos(x, y);
	//set down flags
	switch(button)
	{
		case GLUT_LEFT_BUTTON:
			lb_down = (state==GLUT_UP)?0:1;
			
			if (state == GLUT_UP) //just released the left click.
			{
				//Check for any button clicked:
				for (int i = 0; i < buttons.capacity(); i++)
				{
					if (buttons[i].isInside(mPos))
					{
						buttons[i].BtnClick();
						break;
					}
				}
			}
			break;
		case GLUT_RIGHT_BUTTON:
			rb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_MIDDLE_BUTTON:
			mb_down = (state==GLUT_UP)?0:1;	
			break;
	}
	
#ifdef _DEBUG
	//std::cout << "Mouse callback called. (x, y):" << mPos << endl;
#endif
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;

	// update last x,y
	last_x=x;
	last_y=y;

#ifdef _DEBUG
	vec2 mPos(x, y);
	//std::cout << "motion callback called. (x, y):" << mPos << endl;
#endif

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
				scene->loadOBJModel(s);
			}
			break;
	}
}

void selectMenu(int id)
{
	switch (id)
	{
	case SELECT_OBJECT:
		//TODO
#ifdef _DEBUG
		cout << "Object selection" << endl;
#endif
		break;
	case SELECT_CAMERA:
		//TODO
#ifdef _DEBUG
		cout << "Camera selection" << endl;
#endif
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
	glutAddMenuEntry("Open..", MenuEntry{ FILE_OPEN });
	
	int menuSelect = glutCreateMenu(selectMenu);
	glutAddMenuEntry("Object...", MenuEntry{ SELECT_OBJECT });
	glutAddMenuEntry("Camera...", MenuEntry{ SELECT_CAMERA });

	glutCreateMenu(mainMenu);
	glutAddSubMenu("File", menuFile);
	glutAddSubMenu("Selection", menuSelect);
	glutAddMenuEntry("Demo", MenuEntry{ MAIN_DEMO });
	glutAddMenuEntry("About", MenuEntry{ MAIN_ABOUT });
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
}

void initButtons()
{
	//Use this func to add all GUI buttons

	Button* b1 = new Button(vec2(10, 10), vec2(30, 30));
	b1->SetCallbackFunc(btn_b1_clicked);

	buttons.push_back(*b1);
	
}

#ifdef _DEBUG
void debug_PlayWithVectors()
{
	vec2 a = vec2(1);
	vec2 b = vec2(3, 4);
	cout << "debug_PlayWithVectors started" << endl;

	cout << "2d vector a = " << a << endl;
	cout << "2d vector b = " << b << endl;
	cout << "2d vector a+b = " << a + b << endl;
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

	cout << endl << endl; // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	cout << endl << "Test 1.1: mat2 operator*" << endl;
	mat2 m2a = mat2(1, 2, 3, 4);
	mat2 m2b = mat2(1, 2, 0, 1);
	cout << "m2a: " << m2a;
	cout << "m2b: " << m2b;
	cout << "result m2a * m2b: " << m2a * m2b;
	cout << "result m2b * m2a: " << m2b * m2a;

	cout << endl << "Test 1.2: mat2 operator*=" << endl;
	m2b *= m2b;
	cout << "result m2b*=m2b: " << m2b;

	cout << endl << "Test 1.3: mat2 transpose" << endl;
	cout << "result m2a^T:" << transpose(m2a);

	cout << endl << endl; // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	cout << endl << "Test 2.1: mat3 m*v:" << endl;
	vec3 v3a = vec3(1, 2, 3);
	mat3 m3a = mat3(1, 2, 3,
		4, 5, 6,
		7, 8, 9);
	cout << "m3a: " << m3a << endl;
	cout << "v3a: " << endl << v3a << endl;
	cout << "m3a * v3a : " << endl << m3a * v3a << endl;

	cout << endl << "Test 2.2: mat3 operator*" << endl;
	mat3 m3b = mat3(2); // 2 * Identity
	cout << "m3b: " << m3b << endl;
	cout << "result m3a * m3b: " << m3a * m3b << endl;
	cout << "result m3b * m3a: " << m3b * m3a << endl;

	cout << endl << "Test 2.3: m3a transpose" << endl;
	cout << "result m3a^T:" << transpose(m3a) << endl;

	cout << endl << endl; // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	cout << endl << "Test 3.1: mat4 m*v:" << endl;
	vec4 v4a = vec4(1, 2, 3, 4);
	mat4 m4a = mat4(1, 2, 3, 4,
		5, 6, 7, 8,
		9, 10, 11, 12,
		13, 14, 15, 16);
	cout << "m4a: " << m4a << endl;
	cout << "v4a: " << endl << v4a << endl;
	cout << "m4a * v4a : " << endl << m4a * v4a << endl;

	cout << endl << "Test 3.2: mat4 operator*" << endl;
	mat4 m4b = mat4(2); // 2 * Identity
	cout << "m4b: " << m4b << endl;
	cout << "result m4a * m4b: " << m4a * m4b << endl;
	cout << "result m4b * m4a: " << m4b * m4a << endl;

	cout << endl << "Test 3.3: m4a transpose" << endl;
	cout << "result m4a^T:" << transpose(m4a) << endl;

	cout << endl << endl; // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


	//cout << "Test 4: transformations" << endl;
	//mat4 scale = Scale(2, 2, 2);
	//cout << "Scale mat: " << scale << endl;
	//vec4 v4 = vec4(1, 1, 1, 1);
	//cout << "Scaling by (2,2,2): " << Scale(2,2,2) * v4 << endl;
	//cout << "Scaling by (1,2,3): " << Scale(1, 2, 3) * v4 << endl;

	//mat4 rotate = RotateX(45);
	//cout << "Rotate mat: " << rotate << endl;
	//cout << "Rotating by 45: " << RotateX(45) * v4 << endl;
	//cout << "Rotating by 360: " << RotateX(360) * v4 << endl;
	//cout << "Rotating by 0: " << RotateX(0) * v4 << endl;
	//cout << "Rotating by 180: " << RotateX(180) * v4 << endl;	// BUG? M: result is (1,-1,-1) but not (-1,-1,-1)

	//mat4 trsl = Translate(2, 2, 2);
	//cout << "Translate mat: " << trsl << endl;
	//cout << "Translating by (2,2,2): " << Translate(2, 2, 2) * v4 << endl;
	//cout << "Translating by (0,0,0): " << Translate(0,0,0) * v4 << endl;
	//cout << "Translating by (-0.5,-0.5,0): " << Translate(-0.5, -0.5, 0) * v4 << endl;


	cout << "debug_PlayWithMatrices finished" << endl;
}
#endif

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

	
	
	renderer = new Renderer(512,512);
	scene = new Scene(renderer);
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	initMenu();
	initButtons();

	glutMainLoop();
	delete scene;
	delete renderer;
	return 0;
}

//----------------------------------------------------------------------------
// ---------------------- Don't touch below ----------------------------------
//----------------------------------------------------------------------------


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