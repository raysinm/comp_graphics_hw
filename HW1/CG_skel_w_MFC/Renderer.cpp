#include "StdAfx.h"
#include "Renderer.h"
//#include "CG_skel_w_MFC.h"
#include "CG_skel_w_glfw.h"
#include "InitShader.h"
//#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "GL\freeglut.h"
#include "Button.h"

#define INDEX(width,x,y,c) (x+y*width)*3 + c
#define RED   0
#define GREEN 1
#define BLUE  2

Renderer::Renderer() :m_width(512), m_height(512), m_window(NULL)	// Need to think about default window
{
	InitOpenGLRendering();
	CreateBuffers(m_width, m_height);
}

Renderer::Renderer(int width, int height, GLFWwindow* window) :m_width(width), m_height(height)
{
	m_window = window;
	InitOpenGLRendering();
	CreateBuffers(m_width, m_height);
}

Renderer::~Renderer(void)
{
	if(m_outBuffer)
		delete[] (m_outBuffer);
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
	for (int i = 0; i < 3 * m_width * m_height; i++)
		m_outBuffer[i] = 1.0; //Set all pixels to pure white.
}

void Renderer::SetDemoBuffer()
{
	//vertical line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,256,i,RED)]   =1;
		m_outBuffer[INDEX(m_width,256,i,GREEN)] =0;
		m_outBuffer[INDEX(m_width,256,i,BLUE)]  =0;

	}
	//horizontal line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,i,256,RED)]=0;
		m_outBuffer[INDEX(m_width,i,256,GREEN)]=0;
		m_outBuffer[INDEX(m_width,i,256,BLUE)]=1;

	}
}

void Renderer::DrawBtns()
{
	for (int i = 0; i < buttons.capacity(); i++)
	{
		buttons[i].Draw(m_outBuffer, m_width, m_height);
	}
}







/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	GLenum a = glGetError();
#ifdef _DEBUG
	printf("ERROR in Renderer: %s\n", glewGetErrorString(a));
#endif
	a = glGetError();
#ifdef _DEBUG
	printf("ERROR in Renderer: %s\n", glewGetErrorString(a));
#endif
	glGenTextures(1, &gScreenTex);
	a = glGetError();
#ifdef _DEBUG
	printf("ERROR in Renderer: %s\n", glewGetErrorString(a));
#endif
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[]={
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[]={
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc)+sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	GLint  vPosition = glGetAttribLocation( program, "vPosition" );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0 );

	GLint  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc) );
	glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	//glutSwapBuffers();
	glfwSwapBuffers(m_window);
	a = glGetError();
}