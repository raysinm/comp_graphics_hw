//#include <GL/glew.h>
//#include "FrameBuffer.h"
//
//FrameBuffer::FrameBuffer(float width, float height) : m_width(width), m_height(height)
//{
//	m_outBuffer = new float[(int)(m_width * m_height * 3)];
//	for (int i = 0; i < (int)(3 * m_width * m_height); i++)
//		m_outBuffer[i] = 1.0; //Set all pixels to pure white.
//
//	glGenFramebuffers(1, &fbo);
//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//
//	glGenTextures(1, &texture);
//	glBindTexture(GL_TEXTURE_2D, texture);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, m_outBuffer);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
//
//	glGenRenderbuffers(1, &rbo);
//	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
//
//	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
//
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glBindRenderbuffer(GL_RENDERBUFFER, 0);
//
//
//
//}
//
//FrameBuffer::~FrameBuffer()
//{
//	glDeleteFramebuffers(1, &fbo);
//	glDeleteTextures(1, &texture);
//	glDeleteRenderbuffers(1, &rbo);
//}
//
//unsigned int FrameBuffer::getFrameTexture()
//{
//	return texture;
//}
//
//void FrameBuffer::RescaleFrameBuffer(float width, float height)
//{
//	/* Currently we are overriding the last pixels !!!! */
//	if (width != m_width || height != m_height)
//	{
//		m_width = width;
//		m_height = height;
//		if (m_outBuffer)
//			delete[] m_outBuffer;
//
//		m_outBuffer = new float[(int)(m_width * m_height * 3)];
//		for (int i = 0; i < (int)(3 * m_width * m_height); i++)
//			m_outBuffer[i] = 1.0; //Set all pixels to pure white.
//	}
//
//	glBindTexture(GL_TEXTURE_2D, texture);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, m_outBuffer);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
//
//	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
//}
//
//void FrameBuffer::Bind() const
//{
//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//}
//
//void FrameBuffer::Unbind() const
//{
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}