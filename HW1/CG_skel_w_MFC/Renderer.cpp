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

void Renderer::SetBufferOfModel(vec2* vertecies, unsigned int len, vec4 color)
{
	/*	Each 3 indexes make up a face.
		For example:
		0, 1, 2  - face1
		3, 4, 5  - face2
		6, 7, 8  - face3
		and so on...
	*/

	for (unsigned int i = 0; i < len; i+=3)
	{
		//A   B    C    is the triangle
		//i, i+1, i+2 

		/* Set A to range [0, 1]*/
		vec2 A = vec2( (vertecies[i+0].x + 1) / 2, (vertecies[i+0].y + 1) / 2);
		vec2 B = vec2( (vertecies[i+1].x + 1) / 2, (vertecies[i+1].y + 1) / 2);
		vec2 C = vec2( (vertecies[i+2].x + 1) / 2, (vertecies[i+2].y + 1) / 2);

		/*	Set A_Pxl to range [0, m_wdith - 1]  (X)
							   [0, m_height - 1] (Y)
			Also, keep it in-bound of the screen.
		*/
		vec2 A_Pxl = vec2( max(min(m_width - 1, (A.x * (m_width - 1))) , 0), max(min(m_height - 1,  (A.y * (m_height - 1))), 0));
		vec2 B_Pxl = vec2( max(min(m_width - 1, (B.x * (m_width - 1))) , 0), max(min(m_height - 1,  (B.y * (m_height - 1))), 0));
		vec2 C_Pxl = vec2( max(min(m_width - 1, (C.x * (m_width - 1))) , 0), max(min(m_height - 1,  (C.y * (m_height - 1))), 0));

		
		/* At this point, we have 3 points, in screen space, in-bound */

		/* Draw the 3 lines */
		DrawLine(A_Pxl, B_Pxl, false, color);
		DrawLine(A_Pxl, C_Pxl, false, color);
		DrawLine(B_Pxl, C_Pxl, false, color);
	}
}

void Renderer::SetBufferLines(vec2* points, unsigned int len, vec4 color)
{

	for (unsigned int  i = 0; i < len; i+=2)
	{
		// A B makes the line

		/* Set A to range [0, 1]*/
		vec2 A = vec2((points[i].x + 1) / 2, (points[i].y + 1) / 2);
		vec2 B = vec2((points[i+1].x + 1) / 2, (points[i+1].y + 1) / 2);

		/*	Set A_Pxl to range [0, m_wdith - 1]  (X)
							   [0, m_height - 1] (Y)
			Also, keep it in-bound of the screen.
		*/
		vec2 A_Pxl = vec2(max(min(m_width - 1, (A.x * (m_width - 1))), 0), max(min(m_height - 1, (A.y * (m_height - 1))), 0));
		vec2 B_Pxl = vec2(max(min(m_width - 1, (B.x * (m_width - 1))), 0), max(min(m_height - 1, (B.y * (m_height - 1))), 0));
		

		/* At this point, we have 3 points, in screen space, in-bound */

		/* Draw the line */
		DrawLine(A_Pxl, B_Pxl, false, color);
	}
}

void Renderer::DrawLine(vec2 A, vec2 B, bool isNegative, vec4 color)
{
	if (B.x < A.x)
	{
		DrawLine(B, A, isNegative, color);
		return;
	}
	/* Now we can assume A is left to B*/

	vector<vec2> pixels;
	bool flipXY = false;
	int dy = B.y - A.y;
	int dx = B.x - A.x;
	int y_mul = isNegative ? -1 : 1;

	if (B.y >= A.y) /* Positive Slope */
	{
		if (dy <= dx) /* 0 < Slope < 1*/
		{
			//pixels = ComputePixels_Bresenhams(A, B, false, y_mul);
			ComputePixels_Bresenhams(A, B, false, y_mul, color);
			return;
		}
		else          /* 1 < Slope */
		{
			//pixels = ComputePixels_Bresenhams(A.flip(), B.flip(), true, y_mul); //flip x with y to make it slope < 1
			ComputePixels_Bresenhams(A.flip(), B.flip(), true, y_mul, color); //flip x with y to make it slope < 1
			return;
			//flipXY = true;
		}
	}
	else /* Negative Slope - reflect of X axis */
	{
		DrawLine(vec2(A.x, -A.y), vec2(B.x, -B.y), true, color); // Draw with reflection of X axis.
		return;
	}

	
	//Draw all selected pixels:
	//for (vec2 pix : pixels)
	//{

	//}


}

void Renderer::ComputePixels_Bresenhams(vec2 A, vec2 B, bool flipXY, int y_mul, vec4 color)
{
	if (B.x < A.x)
	{
		ComputePixels_Bresenhams(B, A, flipXY, y_mul, color);
		return;
	}
	/* Now we can assume A is left B*/

	/* Init stuff */
	//vector<vec2> pixels;
	int x = A.x;
	int y = A.y;
	int dx = B.x - A.x;
	int dy = B.y - A.y;
	int d = 2 * dy - dx;
	int de = 2 * dy;
	int dne = 2 * dy - 2 * dx;
	int BX = (int)B.x;
	//pixels.reserve(dx+1);

	for (; x < BX; x++)
	{
		//print (x,y):
		{
			int currentX = x;
			int currentY = y;
			if (flipXY)
			{
				currentX = y;
				currentY = x;
			}

			m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), RED)] = color.x;
			m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), GREEN)] = color.y;
			m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), BLUE)] = color.z;
		}


		if (d < 0)
		{
			d += de;
		}
		else
		{
			y++;
			d += dne;
		}
	}

	//print B:
	{
		int currentX = B.x;
		int currentY = B.y;

		if (flipXY)
		{
			currentX = B.y;
			currentY = B.x;
		}

		m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), RED)] = 0;
		m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), GREEN)] = 0;
		m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), BLUE)] = 0;
	}
	return;
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

		clearBuffer();
	}
}

void Renderer::clearBuffer()
{
	if (m_outBuffer)
	{
		for (int i = 0; i < 3 * m_width * m_height; i++)
			m_outBuffer[i] = 1.0; //Set all pixels to pure white.

	}
}


