#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_glfw.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include "MeshModel.h"

#define INDEX(width,x,y,c) (x+y*width)*3 + c
#define RED   0
#define GREEN 1
#define BLUE  2
#define MAX_Z 65535

Renderer::Renderer(int width, int height, GLFWwindow* window) :
	m_width(width), m_height(height),
	m_max_obj_y(m_height-1), m_min_obj_y(0)
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

	if (m_zbuffer)
		delete[] m_zbuffer;
}

void Renderer::CreateBuffers(int width, int height)
{
	CreateOpenGLBuffer(); //Do not remove this line.

	m_outBuffer = new float[3 * width * height];
	for (UINT i = 0; i < 3 * width * height; i++)
		m_outBuffer[i] = 1.0f; //Set all pixels to pure white.

	m_zbuffer = new UINT[width * height];
	for (UINT i = 0; i < width * height; i++)
		m_zbuffer[i] = MAX_Z;


}

void Renderer::SetBufferLines(const vec2* points, unsigned int len, vec4 color)
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
		

		/* At this point, we have 2 points, in screen space, in-bound */

		/* Draw the line */
		DrawLine(A_Pxl, B_Pxl, false, color);
	}
}

void Renderer::Rasterize_WireFrame(const Vertex* vertices, unsigned int len, vec4 color)
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
		vec2 A = vec2( (vertices[i+0].point.x + 1) / 2, (vertices[i+0].point.y + 1) / 2);
		vec2 B = vec2( (vertices[i+1].point.x + 1) / 2, (vertices[i+1].point.y + 1) / 2);
		vec2 C = vec2( (vertices[i+2].point.x + 1) / 2, (vertices[i+2].point.y + 1) / 2);

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

std::pair<int, int> Renderer::CalcScanlineSpan(Poly& p, int y)
{
	Line scanline = Line(0, y);
	vector<int> intersections_x;
	auto pLines = p.GetLines();

	for (auto line : pLines)
	{
		try
		{
			intersections_x.push_back((scanline.intersect(line)).x);
		}
		catch (ParallelLinesException& e)
		{
			continue;
		}
	}
	sort(intersections_x.begin(), intersections_x.end());	//From smallest to biggest
	
	//*** Find scanline polygon intersection coords (left, right)
	
	int x_left = 0, x_right = (m_width - 1);

	bool left_found = false;
	for (int x : intersections_x)	
	{
		if (x < p.GetMinX())
			continue; // Continue until you find a relevant x inside the polygon
		if (x > p.GetMaxX())
		{	//problem
			//cout << "ERROR: CalcScanlineSpan: x: " << x << " out of range" << endl;
			break;
		}
		if (!left_found)
		{
			x_left = x > 0 ? x : 0;		// If out of range, take 0
			left_found = true;
		}
		else {
			x_right = x < (m_width - 1) ? x : (m_width - 1);	// If out of range, take width of screen
			break;
		}
	}
	// Sanity check:
	if (x_right < x_left)
	{
		cout << "ERROR: CalcScanlineSpan: calculation error in x intersections";
	}

	return std::make_pair(x_left, x_right);

}

vector<Poly> Renderer::CreatePolygonsVector(const MeshModel* model)
{

	//	Get vertices buffer from model
	MeshModel* pModel = (MeshModel*)model;
	Vertex* vertices = pModel->GetBuffer();
	UINT len = pModel->GetBuffer_len(MODEL);
	if (!vertices || len == 0)
		return vector<Poly>();


	vector<vec3>* vnormals = pModel->getVertexNormals();
	vector<vec3>* pFaceNormals = pModel->getFaceNormals();
	vector<Poly> polygons;

	/* Add all polygons to polygons vector */

	for (UINT i = 0; i < len; i += 3)
	{
		// TODO: Implement clipping - Without disfiguring the triangle using the algorithm from clipping tutorial

		///* Set range: [0, 1]  (All dimensions) */
		vec3 A = vec3((vertices[i + 0].point.x + 1) / 2, (vertices[i + 0].point.y + 1) / 2, (vertices[i + 0].point.z + 1) / 2);
		vec3 B = vec3((vertices[i + 1].point.x + 1) / 2, (vertices[i + 1].point.y + 1) / 2, (vertices[i + 1].point.z + 1) / 2);
		vec3 C = vec3((vertices[i + 2].point.x + 1) / 2, (vertices[i + 2].point.y + 1) / 2, (vertices[i + 2].point.z + 1) / 2);

		///*	Set range:   [0, m_width - 1]  (X)
		//				 [0, m_height - 1] (Y)
		//				 [0, MAX_Z]		   (Z)  */
		//vec3 A_Pxl = vec3((UINT)max(min(m_width - 1, (A.x * (m_width - 1))), 0), (UINT)max(min(m_height - 1, (A.y * (m_height - 1))), 0), (UINT)(A.z * MAX_Z));
		//vec3 B_Pxl = vec3((UINT)max(min(m_width - 1, (B.x * (m_width - 1))), 0), (UINT)max(min(m_height - 1, (B.y * (m_height - 1))), 0), (UINT)(B.z * MAX_Z));
		//vec3 C_Pxl = vec3((UINT)max(min(m_width - 1, (C.x * (m_width - 1))), 0), (UINT)max(min(m_height - 1, (C.y * (m_height - 1))), 0), (UINT)(C.z * MAX_Z));
		// 
		
		// !!!****CHANGE: Poly will contain the pixel positions as is, even if outside the screen. Scanline-algo will calculate for each scanline the ACTUAL relevant screen coordinates
		/*vec3 A = vec3(vertices[i + 0].point.x, vertices[i + 0].point.y, vertices[i + 0].point.z);
		vec3 B = vec3(vertices[i + 1].point.x, vertices[i + 1].point.y, vertices[i + 1].point.z);
		vec3 C = vec3(vertices[i + 2].point.x, vertices[i + 2].point.y, vertices[i + 2].point.z);*/

		vec3 A_Pxl = vec3((int)(A.x * (m_width - 1)), (int)(A.y * (m_height - 1)), (UINT)(A.z * MAX_Z));
		vec3 B_Pxl = vec3((int)(B.x * (m_width - 1)), (int)(B.y * (m_height - 1)), (UINT)(B.z * MAX_Z));
		vec3 C_Pxl = vec3((int)(C.x * (m_width - 1)), (int)(C.y * (m_height - 1)), (UINT)(C.z * MAX_Z));


		if (vertices[i].face_index != vertices[i + 1].face_index || vertices[i].face_index != vertices[i + 2].face_index ||
			vertices[i + 1].face_index != vertices[i + 2].face_index)
		{
			/* Should never get here....*/
			/* Keep this just to make sure you and Maya agree on this...*/
			cout << "ERRORRRR!!" << endl;
			return vector<Poly>();
		}

		Poly P = Poly(A_Pxl, \
			B_Pxl, \
			C_Pxl, \
			(*vnormals)[vertices[i + 0].vertex_index], \
			(*vnormals)[vertices[i + 1].vertex_index], \
			(*vnormals)[vertices[i + 2].vertex_index], \
			(*pFaceNormals)[vertices[i].face_index]);

		// Update renderer's min and max Ys
		UpdateMinMaxY(P);

		polygons.push_back(P);
	}

	return polygons;
}



void Renderer::Rasterize_Flat(const MeshModel* model)
{
	if (!model) return; /* Sanity check*/

	/* -------------- 'polygons' vector initialization-------------- */
	vector<Poly> polygons = CreatePolygonsVector(model);
	if (polygons.size() == 0)
	{
		cout << "NOTICE: Rasterize: Polygon vector empty" << endl;
		return;	// Something failed in creation
	}

#ifdef _DEBUG
	// --- CalcScanlineSpan test --- //
	auto vertices = ((MeshModel*)model)->GetBuffer();
	int len = ((MeshModel*)model)->GetBuffer_len(MODEL);
	if (vertices)
		Rasterize_WireFrame(vertices, len);
	
	int y_test = m_height/2;
	auto range = CalcScanlineSpan(polygons[0], y_test);
	cout << "TEST result: " << range.first << ",\t" << range.second << endl;
	
	// --- CalcScanlineSpan test --- //
	cout << "TEST MinMaxY: minY: " << m_min_obj_y << ", maxY: " << m_max_obj_y << endl;
#endif // _DEBUG


	/* -------------- Shading calculation -------------- */


	/* -------------- Scanline-Zbuffer-------------- */
	ScanLineZ_Buffer(polygons);
}


void Renderer::Rasterize_Gouraud(const MeshModel* model)
{
	if (!model) return; /* Sanity check*/

	/* -------------- 'polygons' vector initialization-------------- */
	vector<Poly> polygons = CreatePolygonsVector(model);
	if (polygons.empty())
		return;	// Something failed in creation

	/* -------------- Shading calculation -------------- */

	ScanLineZ_Buffer(polygons);
}

void Renderer::Rasterize_Phong(const MeshModel* model)
{
	if (!model) return; /* Sanity check*/

	/* -------------- 'polygons' vector initialization-------------- */
	vector<Poly> polygons = CreatePolygonsVector(model);
	if (polygons.empty())
		return;	// Something failed in creation

	/* -------------- Shading calculation -------------- */

	ScanLineZ_Buffer(polygons);
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

		m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), RED)]   = color.x;
		m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), GREEN)] = color.y;
		m_outBuffer[INDEX(m_width, currentX, (m_height - (y_mul * currentY) - 1), BLUE)]  = color.z;
	}
	return;
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

void Renderer::ScanLineZ_Buffer(vector<Poly>& polygons)
{
	/* Psudo Code - Z Buffer Scan line algo */


//	Foreach scanline ( YMIN <= y <= YMAX ):
//		let A = { P in polygons if  P.MIN_Y <= y <= P.MAX_Y }
//		Foreach polygon p in A:
//			Foreach pixel (x, y) in p span on scanline (y):
//				let z = Depth(P, x, y)		(Calculate the z value for the pixel. see Lecture 4 page 24)
//				if z < m_zbuffer[x][y]:
//					PutColor(x,y, Col(p))	(Get the color of the pixel (x,y) in polygon p)		// Col(p) Will get Polygon and Lighting sources
//					m_zbuffer[x][y] = z


	// Example for getting scanline range (represented by std::pair)
	//pair scanline = CalcScanlineSpan(p, y);
	//for(UINT i = scanline.first; i <= scanline.second; i++) {
	//......

	//y_range = 
}

void Renderer::CreateTexture()
{
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, m_outBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Renderer::updateTexture()
{
	//Yonatan: It seems to work without all this deleting & generating... better performance without them

	//// Delete the existing texture
	//glDeleteTextures(1, &m_textureID);

	//// Generate a new texture
	//glGenTextures(1, &m_textureID);

	//// Bind the new texture
	//glBindTexture(GL_TEXTURE_2D, m_textureID);

	//// Set texture parameters (adjust as needed)
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Allocate texture storage with the updated buffer data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, m_outBuffer);
}

vec2 Renderer::GetWindowSize()
{
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	return vec2(width, height);
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
	if (m_outBuffer)
	{
		delete[] m_outBuffer;
		m_outBuffer = new float[3 * m_width * m_height];

	}

	if (m_zbuffer)
	{
		delete[] m_zbuffer;
		m_zbuffer = new UINT[m_width * m_height];
	}

	clearBuffer();
}

void Renderer::clearBuffer()
{
	if (m_outBuffer)
	{
		for (int i = 0; i < 3 * m_width * m_height; i++)
			m_outBuffer[i] = 1.0; //Set all pixels to pure white.
	}

	if (m_zbuffer)
	{
		for (UINT i = 0; i < m_width * m_height; i++)
			m_zbuffer[i] = MAX_Z;
	}
}

void Renderer::UpdateMinMaxY(Poly& P)
{
	m_min_obj_y = min(m_height-1, max(0, min(m_min_obj_y, P.GetMinY())));
	m_max_obj_y = max(0, min(m_height - 1, max(m_max_obj_y, P.GetMaxY())));
}

void Renderer::ResetMinMaxY()
{
	m_min_obj_y = 0;
	m_max_obj_y = m_height - 1;
}




/////////////////////////////////////////////////////
//         OpenGL stuff. Don't touch.			   //
/////////////////////////////////////////////////////

void Renderer::InitOpenGLRendering()
{
	GLenum a = glGetError();

	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();

	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[] = {
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[] = {
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1 };
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc) + sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	GLint  vPosition = glGetAttribLocation(program, "vPosition");

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0);

	GLint  vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid*)sizeof(vtc));
	glProgramUniform1i(program, glGetUniformLocation(program, "texture"), 0);
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	//glViewport(0, 0, m_width, m_height);
}