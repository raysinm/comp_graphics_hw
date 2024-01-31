#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_glfw.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3 + c
#define RED   0
#define GREEN 1
#define BLUE  2

Renderer::Renderer(int width, int height, GLFWwindow* window) :m_width(width), m_height(height)
{
	m_window = window;
	//InitOpenGLRendering();	// Probably needed for later
	CreateBuffers(m_width, m_height);
	CreateTexture();
}

Renderer::~Renderer(void)
{
	if(m_outBuffer)
		delete[] (m_outBuffer);
}

void Renderer::CreateBuffers(int width, int height)
{
	CreateOpenGLBuffer(); //Do not remove this line.

	m_outBuffer = new float[3 * m_width * m_height];
	for (int i = 0; i < 3 * m_width * m_height; i++)
		m_outBuffer[i] = 1.0f; //Set all pixels to pure white.
}

void Renderer::SetDemoBuffer()
{

	//vertical line
	for(int i=0; i<m_height; i++)
	{
		int vert_pos = m_width / 2;
		m_outBuffer[INDEX(m_width, vert_pos, i, RED)] = 1;
		m_outBuffer[INDEX(m_width, vert_pos, i, GREEN)] = 0;
		m_outBuffer[INDEX(m_width, vert_pos, i, BLUE)] = 0;

	}

	//horizontal line
	for(int i=0; i<m_width; i++)
	{
		int horz_pos = m_height / 2;
		m_outBuffer[INDEX(m_width, i, horz_pos, RED)] = 0;
		m_outBuffer[INDEX(m_width,i, horz_pos,GREEN)]=0;
		m_outBuffer[INDEX(m_width,i, horz_pos,BLUE)]=1;

	}
}

void Renderer::SetBufferOfModel(vec2* vertecies)
{
}






/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	GLenum a = glGetError();

	a = glGetError();
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
	//glTexSubImage2D(/GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glfwSwapBuffers(m_window);
	a = glGetError();
}

void Renderer::CreateTexture()
{
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, m_outBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Renderer::updateTexture() {
	// Delete the existing texture
	glDeleteTextures(1, &m_textureID);

	// Generate a new texture
	glGenTextures(1, &m_textureID);

	// Bind the new texture
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	// Set texture parameters (adjust as needed)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Allocate texture storage with the updated buffer data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, m_outBuffer);
}

vec2 Renderer::GetBufferSize()
{
	return vec2(m_width, m_height);
}

void Renderer::update(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		cout << "Debug: update called!" << endl;
		m_width = width;
		m_height = height;
		updateBuffer();
		updateTexture();
	}
}

void Renderer::updateBuffer()
{
	// reallocate buffer if needed

	if (m_outBuffer)
	{
		delete[] m_outBuffer;
		m_outBuffer = new float[3 * m_width * m_height];
		for (int i = 0; i < 3 * m_width * m_height; i++)
			m_outBuffer[i] = 1.0; //Set all pixels to pure white.

	}
}