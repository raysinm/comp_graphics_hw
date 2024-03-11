#include <oneapi/tbb.h>
#include "Renderer.h"
#include "CG_skel_w_glfw.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include "MeshModel.h"


extern Scene* scene;

Renderer::Renderer(int width, int height, GLFWwindow* window) :
	m_width(width), m_height(height)
{
	m_window = window;
	//InitOpenGLRendering();	// Probably needed for later
	CreateBuffers(m_width, m_height);
	CreateTexture();

	kernel = createGaussianKernel(10, 1);
}

Renderer::~Renderer(void)
{
	if(m_outBuffer)
		delete[] (m_outBuffer);

	if (m_zbuffer)
		delete[] m_zbuffer;

	if (m_outBuffer_fsblur)
		delete[] m_outBuffer_fsblur;
}

void Renderer::CreateBuffers(int width, int height)
{
	CreateOpenGLBuffer(); //Do not remove this line.

	m_outBuffer			= new float[3 * width * height];
	m_outBuffer_fsblur	= new float[3 * width * height];
	m_zbuffer			= new UINT[width * height];
	
	for (UINT i = 0; i < 3 * width * height; i++) {
		m_outBuffer[i] = 0;
		m_outBuffer_fsblur[i] = 0;
	}

	for (UINT i = 0; i < width * height; i++)
		m_zbuffer[i] = MAX_Z;
}

void Renderer::SetBufferLines(const vec2* points, unsigned int len)
{
	SetBufferLines(points, len, DEFAULT_WIREFRAME_COLOR);
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

void Renderer::Rasterize_WireFrame(const Vertex* vertices, unsigned int len)
{
	Rasterize_WireFrame(vertices, len, DEFAULT_WIREFRAME_COLOR);
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
	set<int> intersections_x;
	auto pLines = p.GetLines();

	bool same_line_as_y = true;
	for (auto line : pLines)
	{
		same_line_as_y &= (line == scanline);
		bool is_par = false;
		vec2 interPoint = scanline.intersect(line, &is_par);
		if (is_par)
			continue;

		float upperY = max(line.getA().y, line.getB().y);
		float lowerY = min(line.getA().y, line.getB().y);

		if(lowerY <= interPoint.y && interPoint.y <= upperY)
			intersections_x.insert(round(interPoint.x));		// round is needed for handling intersection points falling within the same pixel
	}
	if (same_line_as_y)
	{
		// The polygon looks line a line, take min and max x as scanline span
		intersections_x.clear();
		intersections_x.insert(p.GetMinX());
		intersections_x.insert(p.GetMaxX());
	}


	std::vector<int> intersectionsSorted (intersections_x.begin(), intersections_x.end());

	sort(intersectionsSorted.begin(), intersectionsSorted.end());	//From smallest to biggest


	if (intersectionsSorted.size() == 1)
		intersectionsSorted.push_back(intersectionsSorted[0]); //duplicate the same x coord to make it a pair of itself.
	
	//*** Find scanline polygon intersection coords (left, right)
	
	int x_left=0, x_right=m_width-1;

	bool left_found = false;
	for (int x : intersectionsSorted)
	{
		if (x < p.GetMinX())
			continue; // Continue until you find a relevant x inside the polygon

		if (!left_found)
		{
			x_left = max(x, 0);		// If out of range, take 0
			left_found = true;
		}
		else
		{
			x_right = max(0, min(x, m_width - 1));
			break;
		}
	}

	return std::make_pair(x_left, x_right);

}

vector<Poly> Renderer::CreatePolygonsVector(const MeshModel* model)
{
	MeshModel* pModel = (MeshModel*)model;
	Vertex* vertices = pModel->GetBuffer();
	UINT len = pModel->GetBuffer_len(MODEL);
	if (!vertices || len == 0)
		return vector<Poly>();


	vector<vec3>* vnormals = pModel->getVertexNormalsViewSpace();
	vector<vec3>* pFaceNormals = pModel->getFaceNormalsViewSpace();
	vector<Poly> polygons;

	ResetMinMaxY();
	/* Add all polygons to polygons vector */
	for (UINT i = 0; i < len; i += 3)
	{
		/* Set range: [0, 1]  (All dimensions) */
		vec3 A = vec3((vertices[i + 0].point.x + 1) / 2, (vertices[i + 0].point.y + 1) / 2, (vertices[i + 0].point.z + 1) / 2);
		vec3 B = vec3((vertices[i + 1].point.x + 1) / 2, (vertices[i + 1].point.y + 1) / 2, (vertices[i + 1].point.z + 1) / 2);
		vec3 C = vec3((vertices[i + 2].point.x + 1) / 2, (vertices[i + 2].point.y + 1) / 2, (vertices[i + 2].point.z + 1) / 2);

		//	Set range:   [0, m_width - 1]  (X)
		//				 [0, m_height - 1] (Y)
		//				 [0, MAX_Z]		   (Z)
		vec3 A_Pxl = vec3((int)(A.x * (m_width - 1)), (int)(A.y * (m_height - 1)), (UINT)(A.z * MAX_Z));
		vec3 B_Pxl = vec3((int)(B.x * (m_width - 1)), (int)(B.y * (m_height - 1)), (UINT)(B.z * MAX_Z));
		vec3 C_Pxl = vec3((int)(C.x * (m_width - 1)), (int)(C.y * (m_height - 1)), (UINT)(C.z * MAX_Z));


		Poly P = Poly(	A_Pxl,										\
						B_Pxl,										\
						C_Pxl,										\
						(*vnormals)[vertices[i + 0].vertex_index],	\
						(*vnormals)[vertices[i + 1].vertex_index],	\
						(*vnormals)[vertices[i + 2].vertex_index],	\
						(*pFaceNormals)[vertices[i].face_index],	\
						pModel->isUniformMaterial,					\
						pModel->getMaterials(), 					\
						pModel->getUserDefinedMaterial(), 			\
						vertices[i + 0].vertex_index,				\
						vertices[i + 1].vertex_index,				\
						vertices[i + 2].vertex_index,				\
						vertices[i + 0].point_cameraspace,			\
						vertices[i + 1].point_cameraspace,			\
						vertices[i + 2].point_cameraspace );
		

		UpdateMinMaxY(P);
		polygons.push_back(P);
	}

	return polygons;
}

void Renderer::Rasterize_Flat(const MeshModel* model)
{
	if (!model) return; /* Sanity check*/
	vector<Poly> polygons = CreatePolygonsVector(model);
	if (polygons.size() == 0)
	{
		return;	// Something failed in creation
	}

	ScanLineZ_Buffer(polygons);
}

void Renderer::Rasterize_Gouraud(const MeshModel* model)
{
	if (!model) return; /* Sanity check*/
	vector<Poly> polygons = CreatePolygonsVector(model);
	if (polygons.size() == 0)
	{
		return;	// Something failed in creation
	}

	ScanLineZ_Buffer(polygons);
}

void Renderer::Rasterize_Phong(const MeshModel* model)
{

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
			ComputePixels_Bresenhams(A, B, false, y_mul, color);
			return;
		}
		else          /* 1 < Slope */
		{
			ComputePixels_Bresenhams(A.flip(), B.flip(), true, y_mul, color); //flip x with y to make it slope < 1
			return;
		}
	}
	else /* Negative Slope - reflect of X axis */
	{
		DrawLine(vec2(A.x, -A.y), vec2(B.x, -B.y), true, color); // Draw with reflection of X axis.
		return;
	}
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

			m_outBuffer[Index(m_width, currentX, (m_height - (y_mul * currentY) - 1), RED)] = color.x;
			m_outBuffer[Index(m_width, currentX, (m_height - (y_mul * currentY) - 1), GREEN)] = color.y;
			m_outBuffer[Index(m_width, currentX, (m_height - (y_mul * currentY) - 1), BLUE)] = color.z;
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

		m_outBuffer[Index(m_width, currentX, (m_height - (y_mul * currentY) - 1), RED)]   = color.x;
		m_outBuffer[Index(m_width, currentX, (m_height - (y_mul * currentY) - 1), GREEN)] = color.y;
		m_outBuffer[Index(m_width, currentX, (m_height - (y_mul * currentY) - 1), BLUE)]  = color.z;
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

	tbb::parallel_for(m_min_obj_y, m_max_obj_y, [&](int y)
		{
			//for (auto y = m_min_obj_y; y <= m_max_obj_y; y++)
			for (auto P : polygons)
			{
				////========= FOR DEBUG
				//vec3 color;	
				//switch (P.id)
				//{
				//case(0):
				//	color = vec3(0.25, 0.25, 0.25);	// grey
				//	break;
				//case(1):
				//	color = vec3(1, 0, 0);
				//	break;
				//case(2):
				//	color = vec3(0, 1, 0);
				//	break;
				//case(3):
				//	color = vec3(0, 0, 1);
				//	break;
				//}
				////=====================
				if (P.GetMinY() > y || P.GetMaxY() < y)
					continue;

				std::pair<int, int> scan_span = CalcScanlineSpan(P, y);
				for (int x = scan_span.first; x <= scan_span.second; x++)
				{
					UINT z = P.Depth(x, y);
					int z_index = Z_Index(m_width, x, y);
					UINT prevValue = m_zbuffer[z_index];
					if (z <= prevValue)
					{
						vec3 color = GetColor(vec3(x, y, z), P);
						if (scene->applyFog)
							color = scene->fog->applyFog(z, color);
						PutColor(x, y, color);
						m_zbuffer[z_index] = z;
					}
				}
			}
		});
}

void Renderer::PutColor(UINT x, UINT y, vec3& color)
{
	m_outBuffer[Index(m_width, x, (m_height - y - 1), RED)]   = color.x;
	m_outBuffer[Index(m_width, x, (m_height - y - 1), GREEN)] = color.y;
	m_outBuffer[Index(m_width, x, (m_height - y - 1), BLUE)]  = color.z;

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

void Renderer::ApplyBloomFilter()
{
	int NUM_OF_WEIGHTS = kernel.size();
	int halfKernelSize = (int)(NUM_OF_WEIGHTS / 2);

	for (UINT x = 0; x < m_width; x++)
	{
		for (UINT y = 0; y < m_height; y++)
		{
			vec3 value = vec3(0);
			value.x = m_outBuffer[Index(m_width, x, y, RED)];
			value.y = m_outBuffer[Index(m_width, x, y, GREEN)];
			value.z = m_outBuffer[Index(m_width, x, y, BLUE)];

			if (value.sum() > bloom_filter_threshold)
			{
				highlightPixels[vec2(x, y)] = value;
				for (int dx = -2; dx < 2; dx++)
				{
					for (int dy = -2; dy < 2; dy++)
					{
						if (dx == 0 && dy == 0) continue;
						if (x + dx < 0 || x + dx >= m_width || y + dy < 0 || y + dy >= m_height) continue;
						auto it = highlightPixels.find(vec2(x + dx, y + dy));
						if (it != highlightPixels.end()) continue;
						
						highlightPixels.insert({ vec2(x + dx,y + dy), vec3(0) });
					}
				}
			}
		}
	}

	for (const auto& pair : highlightPixels)
	{
		const vec2& pixel = pair.first;
		const vec3& value = pair.second;
		
		vec3 result = kernel[0] * value;

		// Apply the gauissian blurr, save the result.
		for (int t = 1; t < NUM_OF_WEIGHTS; t++)
		{
			/* Right */
			auto it = highlightPixels.find(vec2(pixel.x + t, pixel.y));
			if (it != highlightPixels.end())
				result += kernel[t] * (it->second);


			/* Left*/
			it = highlightPixels.find(vec2(pixel.x - t, pixel.y));
			if (it != highlightPixels.end())
				result += kernel[t] * (it->second);

			
			/* Up */
			it = highlightPixels.find(vec2(pixel.x, pixel.y - t));
			if (it != highlightPixels.end())
				result += kernel[t] * (it->second);


			/* Down */
			it = highlightPixels.find(vec2(pixel.x, pixel.y + t));
			if (it != highlightPixels.end())
				result += kernel[t] * (it->second);
		}

		// Apply the multiplier factor
		result *= bloom_filter_factor;

		// Add the result to the scene
		for (int c = 0; c < 3; c++)
			m_outBuffer[Index(m_width, pixel.x, pixel.y, c)] += result[c];
	}
	
	highlightPixels.clear();
}

void Renderer::ApplyFullScreenBlur()
{
	if (fs_blur_iterations == 0) return;

	for (int i = 0; i < fs_blur_iterations; i++)
	{
		gaussianBlur(m_outBuffer, m_outBuffer_fsblur, 1.0f);
	}


	for (int i = 0; i < m_width * m_height * 3; i++)
	{
		m_outBuffer[i] = m_outBuffer_fsblur[i];
	}
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

	if (m_outBuffer_fsblur)
	{
		delete[] m_outBuffer_fsblur;
		m_outBuffer_fsblur = new float[3 * m_width * m_height];
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
			m_outBuffer[i] = DEFAULT_BACKGROUND_COLOR;
	}

	if (m_outBuffer_fsblur)
	{
		for (int i = 0; i < 3 * m_width * m_height; i++)
			m_outBuffer_fsblur[i] = 0;
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
	m_max_obj_y = min(m_height-1, max(0, max(m_max_obj_y, P.GetMaxY())));
}

void Renderer::ResetMinMaxY()
{
	m_max_obj_y = 0;
	m_min_obj_y = m_height-1;
}

void Renderer::calcIntensity(Light* lightSource, vec3& Ia_total, vec3& Id_total, vec3& Is_total, vec3& P, vec3& N, vec3& V, Material& mate)
{
	vec3 I = lightSource->getDirectionCameraSpace();	// Light source direction to P (Assume Parallel light source)
	vec3 R = normalize(I - (2 * dot(I, N) * N));		// Direction of reflected light

	if (lightSource->getLightType() == AMBIENT_LIGHT)
	{
		Ia_total += lightSource->La * mate.Ka * lightSource->getColor();
	}
	else /* Parallel or Point light source*/
	{
		/* Recalculate the I and R because it was calculated for parallel light source */
		if (lightSource->getLightType() == POINT_LIGHT)
		{
			I = normalize(lightSource->getPositionCameraSpace() - P);
			R = normalize(I - (2 * dot(I, N) * N));	// Direction of reflected light
		}

		/* Calcualte diffuse */
		float dotProduct_IN = max(0, dot(I, N));
		Id_total += (lightSource->Ld * mate.Kd * dotProduct_IN) * (lightSource->getColor() * mate.getDiffuse());

		/* Calcualte specular */
		float dotProduct_RV = max(0, dot(R, V));
		pow(dotProduct_RV, mate.COS_ALPHA);
		Is_total += (lightSource->Ls * mate.Ks * dotProduct_RV) * (lightSource->getColor() * mate.getSpecular());
	}
}

vec3 Renderer::GetColor(vec3& pixl, Poly& p)
{
	vec3 Ia_total = vec3(0);
	vec3 Id_total = vec3(0);
	vec3 Is_total = vec3(0);

	if (scene->draw_algo == FLAT)
	{
		if (p.FLAT_calculatedColor)
			return p.FLAT_calculatedColorValue;


		vec3 P = p.GetCenter();								// Point in camera space
		vec3 N = p.GetFaceNormal();							// Normal of the polygon
		vec3 V = normalize(-P);								// Direction to COP (center of projection === camera)

		Material& mate = p.InterpolateMaterial(P);

		for (auto lightSource : scene->lights)
		{
			calcIntensity(lightSource, Ia_total, Id_total, Is_total, P, N, V, mate);
		}

		/* Add emissive light INDEPENDENT to any light source*/
		Ia_total += mate.EmissiveFactor * mate.c_emissive;

		p.FLAT_calculatedColor = true;
		p.FLAT_calculatedColorValue = (Ia_total + Id_total + Is_total).clamp(0,1);

		return p.FLAT_calculatedColorValue;
	}
	else if (scene->draw_algo == GOURAUD)
	{
		if (!p.GOUROD_calculatedColors)
		{
			for (int vert = 0; vert < 3; vert++)
			{
				Ia_total = vec3(0);
				Id_total = vec3(0);
				Is_total = vec3(0);
				
				vec3 P = p.GetPoint(vert);							// Point in camera space
				vec3 N = p.GetVN(vert);								// Normal of the polygon
				vec3 V = normalize(-P);								// Direction to COP (center of projection === camera)

				Material mate = p.InterpolateMaterial(P);
				for (auto lightSource : scene->lights)
				{
					calcIntensity(lightSource, Ia_total, Id_total, Is_total, P, N, V, mate);
				}

				/* Add emissive light INDEPENDENT to any light source*/
				Ia_total += mate.EmissiveFactor * mate.c_emissive;
				
				p.GOUROD_colors[vert] = (Ia_total + Id_total + Is_total).clamp(0, 1);
			}

			p.GOUROD_calculatedColors = true;
		}

		return p.GOUROD_interpolate(vec2(pixl.x, pixl.y));

	}
	else if (scene->draw_algo == PHONG)
	{

	}
	
	return vec3(0);
}

// Function to apply Gaussian blur to an image
void Renderer::gaussianBlur(const float* image, float* blurredImage, const float factor)
{
	int NUM_OF_WEIGHTS = kernel.size();
	int halfKernelSize = (int)(NUM_OF_WEIGHTS / 2);

	// Convolve the image with the kernel
	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			vec3 res = vec3(0);
			for (int horizontal = 0; horizontal < 2; horizontal++)
			{
				for (int i = -halfKernelSize; i < halfKernelSize; ++i)
				{
					int newX;
					int newY;
					if (horizontal == 1)
					{
						if (i == 0) continue;
						newX = max(0, min(m_width - 1, x + i));
						newY = y;
					}
					else
					{
						newX = x;
						newY = max(0, min(m_height - 1, y + i));
					}

					for (int c = 0; c < 3; c++)
						res[c] += image[Index(m_width, newX, newY, c)] * kernel[i + halfKernelSize];
				}
			}

			for (int c = 0; c < 3; c++)
				blurredImage[Index(m_width, x, y, c)] += res[c] * factor / 2;
		}
	}
}

std::vector<float> Renderer::createGaussianKernel(int size, float sigma)
{
	std::vector<float> kernel(size);
	float sum = 0.0f;
	int halfSize = size / 2;
	for (int i = -halfSize; i < halfSize; ++i) {
		kernel[i + halfSize] = exp(-(i * i) / (2 * sigma * sigma)) / (sqrt(2 * M_PI) * sigma);
		sum += kernel[i + halfSize];
	}
	// Normalize the kernel
	for (int i = 0; i < size; ++i) {
		kernel[i] /= sum;
	}
	return kernel;
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