#include "pch.h"
#include "GL/glew.h"
#include "FrameRenderBuffer.h"
#include "FramebufferObject.h"
#include "RenderBuffer.h"
#include "Shader.h"

#pragma warning( disable : 4996)
namespace
{
	const GLchar* postvert = STRINGIFY(
		#version 410\n

		// vertex positions input attribute
		layout(location = 0) in vec2 position;

		// texture coordinates to be interpolated to fragment shaders
		out vec2 st;

		void main() {
			// interpolate texture coordinates
			st = (position + 1.0) * 0.5;
			// transform vertex position to clip space (camera view and perspective)
			gl_Position = vec4(position, 0.0, 1.0);
		}
	);

	const GLchar* postfrag = STRINGIFY(
		#version 410\n

		// texture coordinates from vertex shaders
		in vec2 st;

		//uniform samplerRect texsample;
		uniform sampler2D texsample;

		layout(location = 0) out vec4 frag_colour;
		void main() {
			vec3 colour;
			colour = texture(texsample, st).rgb;
			frag_colour = vec4(colour,1.0);
		}

	);

	bool SaveImage(char* szPathName, void* lpBits, int w, int h)
	{
		//Create a new file for writing
		FILE *pFile = fopen(szPathName, "wb");

		if (pFile == NULL){
			return false;
		}

		BITMAPINFOHEADER BMIH;
		BMIH.biSize = sizeof(BITMAPINFOHEADER);
		BMIH.biSizeImage = w * h * 3;
		// Create the bitmap for this OpenGL context
		BMIH.biSize = sizeof(BITMAPINFOHEADER);
		BMIH.biWidth = w;
		BMIH.biHeight = h;
		BMIH.biPlanes = 1;
		BMIH.biBitCount = 24;
		BMIH.biCompression = BI_RGB;
		BMIH.biSizeImage = w * h * 3;
		BMIH.biXPelsPerMeter = 0;//CID 19539
		BMIH.biYPelsPerMeter = 0;//CID 19539
		BMIH.biClrUsed = 0;//CID 19539
		BMIH.biClrImportant = 0;//CID 19539
		BITMAPFILEHEADER bmfh;
		int nBitsOffset = sizeof(BITMAPFILEHEADER) + BMIH.biSize;
		LONG lImageSize = BMIH.biSizeImage;
		LONG lFileSize = nBitsOffset + lImageSize;
		bmfh.bfType = 'B' + ('M' << 8);
		bmfh.bfOffBits = nBitsOffset;
		bmfh.bfSize = lFileSize;
		bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
		//Write the bitmap file header
		size_t nWrittenFileHeaderSize = fwrite(&bmfh, (size_t)1,sizeof(BITMAPFILEHEADER), pFile);

		//And then the bitmap info header

		size_t nWrittenInfoHeaderSize = fwrite(&BMIH, (size_t)1, sizeof(BITMAPINFOHEADER), pFile);

		//Finally, write the image data itself
		//-- the data represents our drawing
		size_t nWrittenDIBDataSize = fwrite(lpBits, (size_t)1, lImageSize, pFile);
		fclose(pFile);
		return true;
	}
};

CFrameRenderBuffer::CFrameRenderBuffer()
{
}

CFrameRenderBuffer::~CFrameRenderBuffer()
{
	if (m_pColorBuffer != NULL)
	{
		delete[] m_pColorBuffer;
	}
	if (m_pDepthBuffer != NULL)
	{
		delete[] m_pDepthBuffer;
	}
	clear();
}

void CFrameRenderBuffer::clear()
{
	if (m_nScreenVAO > 0) {
		glDeleteVertexArrays(1, &m_nScreenVAO);
		m_nScreenVAO = 0;
	}
	if (m_nFrameBufferTex > 0) {
		glDeleteTextures(1, &m_nFrameBufferTex);
		m_nFrameBufferTex = 0;
	}

	if (m_nFrameBufferObject > 0) {
		glDeleteFramebuffers(1, &m_nFrameBufferObject);
		m_nFrameBufferObject = 0;
	}

	if (m_nDepthbufferObject > 0) {
		glDeleteFramebuffers(1, &m_nDepthbufferObject);
		m_nDepthbufferObject = 0;
	}
	if (m_nStencilbufferObject > 0) {
		glDeleteFramebuffers(1, &m_nStencilbufferObject);
		m_nStencilbufferObject = 0;
	}
}


BOOL CFrameRenderBuffer::GLCreateFBO(int cx, int cy)
{
	if (m_bRender) {
		if (!m_pTextureShader) {
			m_pTextureShader = std::make_unique<Shader>();
			m_pTextureShader->LoadFromString(postvert, postfrag, 0);
		}
		GLfloat ss_quad_pos[] = { -1.0, -1.0, 1.0,	-1.0, 1.0,	1.0,
			1.0,	1.0,	-1.0, 1.0,	-1.0, -1.0 };
		// create VBOs and VAO in the usual way
		glGenVertexArrays(1, &m_nScreenVAO);
		glBindVertexArray(m_nScreenVAO);
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ss_quad_pos), ss_quad_pos,
			GL_STATIC_DRAW);

		auto loc = glGetAttribLocation(m_pTextureShader->GetShaderObject(), "position");
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
		glBindVertexArray(0);
	}

	m_bInitial = FALSE;

	glGenFramebuffers(1, &m_nFrameBufferObject);
	glGenTextures(1, &m_nFrameBufferTex);
	glBindTexture(m_nTextureUnit, m_nFrameBufferTex);
	glTexImage2D(m_nTextureUnit, 0, GL_RGBA32F, cx, cy, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(m_nTextureUnit, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(m_nTextureUnit, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(m_nTextureUnit, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(m_nTextureUnit, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, m_nFrameBufferObject);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_nTextureUnit, m_nFrameBufferTex, 0);

	glGenRenderbuffers(1, &m_nDepthbufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthbufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, cx, cy);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		m_nDepthbufferObject);

	if (m_FrameBufferType == CFrameBufferType::FR_STENCIL_BUFFER) {
		glGenRenderbuffers(1, &m_nStencilbufferObject);
		glBindRenderbuffer(GL_RENDERBUFFER, m_nStencilbufferObject);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_COMPONENTS, cx, cy);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_nStencilbufferObject);
	}

	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_bufs);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (!CheckFrameBuffer())
		return FALSE;

	m_bInitial = TRUE;
	return TRUE;
}

bool CFrameRenderBuffer::GLProcessFBO(int cx, int cy, bool bUseCapture)
{
	m_bCapture = bUseCapture;
	if (!CheckHardware()){
		GLRemoveFBO();
		return false;
	}

	if (HasFBO()){
		if (IsSizeChanged(cx, cy)){
			if (GLResizeFBO(cx, cy)){
				m_size.SetSize(cx, cy);
				return true;
			}
			else{
				GLRemoveFBO();
				m_size.SetSize(0, 0);
				return false;
			}
		}
	}
	else{
		if (GLCreateFBO(cx, cy)){
			m_size.SetSize(cx, cy);
			return true;
		}
		else{
			GLRemoveFBO();
			m_size.SetSize(0, 0);
			return false;
		}
	}
	return false;
}

void CFrameRenderBuffer::GLBeginFBO()
{
	if (m_FrameBufferType == CFrameBufferType::FR_COLOR_BUFFER){
		if (HasFBO()){
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_ncurrentFB);
			glBindFramebuffer(GL_FRAMEBUFFER, m_nFrameBufferObject);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_DEPTH_BUFFER) {
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_STENCIL_BUFFER) {
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_DEPTH_STENCIL_BUFFER) {
	}
}

void CFrameRenderBuffer::GLEndFBO()
{
	if (m_FrameBufferType == CFrameBufferType::FR_COLOR_BUFFER) {
		if (HasFBO()) {
			glBindFramebuffer(GL_FRAMEBUFFER, m_ncurrentFB);
		}
		GLRestoreFBO();
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_DEPTH_BUFFER) {
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_STENCIL_BUFFER) {
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_DEPTH_STENCIL_BUFFER) {
	}

}

void CFrameRenderBuffer::bind()
{
	if (m_FrameBufferType == CFrameBufferType::FR_COLOR_BUFFER) {
		if (HasFBO()) {
			glBindFramebuffer(GL_FRAMEBUFFER, m_nFrameBufferObject);
		}
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_DEPTH_BUFFER) {
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_STENCIL_BUFFER) {
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_DEPTH_STENCIL_BUFFER) {
	}
}

void CFrameRenderBuffer::unbind()
{
	if (m_FrameBufferType == CFrameBufferType::FR_COLOR_BUFFER) {
		if (HasFBO()) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_DEPTH_BUFFER) {
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_STENCIL_BUFFER) {
	}
	else if (m_FrameBufferType == CFrameBufferType::FR_DEPTH_STENCIL_BUFFER) {
	}
}

BOOL CFrameRenderBuffer::HasFBO()
{
	return (m_nFrameBufferObject != 0 && CheckFrameBuffer());
}

void CFrameRenderBuffer::GLRemoveFBO()
{
	clear();
}

void CFrameRenderBuffer::DrawTexture(GLuint texId)
{
	if (m_pTextureShader && m_pTextureShader->IsValid())
		m_pTextureShader->Enable();

	// activate the first texture slot and put texture from previous pass in it
	glActiveTexture(GL_TEXTURE0);
	glEnable(m_nTextureUnit);
	glBindTexture(m_nTextureUnit, texId);
	auto loc = m_pTextureShader->GetUniformLocation("texsample");
	if (loc >= 0) {
		glUniform1i(loc, 0);
	}
	// draw the quad that covers the screen area
	glBindVertexArray(m_nScreenVAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_nTextureUnit, 0);
	glDisable(m_nTextureUnit);

	if (m_pTextureShader && m_pTextureShader->IsValid())
		m_pTextureShader->Disable();
}

void CFrameRenderBuffer::GetPixel(int posx, int posy, float pData[4])
{
	bind();
	glReadPixels(posx, posy, 1, 1, GL_RGBA, GL_FLOAT, pData);
	unbind();
}

void CFrameRenderBuffer::GLRestoreFBO()
{
	if (HasFBO())
	{
		if (m_bRender) {
			if (m_pTextureShader && m_pTextureShader->IsValid())
				m_pTextureShader->Enable();

			glBindVertexArray(m_nScreenVAO);
			// activate the first texture slot and put texture from previous pass in it
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(m_nTextureUnit, m_nFrameBufferTex);
			auto loc = m_pTextureShader->GetUniformLocation("texsample");
			if (loc >= 0) {
				glUniform1i(loc, 0);
			}
			// draw the quad that covers the screen area
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glBindVertexArray(0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(m_nTextureUnit, 0);
			glDisable(m_nTextureUnit);

			if (m_pTextureShader && m_pTextureShader->IsValid())
				m_pTextureShader->Disable();
		}
	}
}

BOOL CFrameRenderBuffer::IsSizeChanged(int cx, int cy)
{
	if (m_size.cx == cx && m_size.cy == cy)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CFrameRenderBuffer::GLResizeFBO(int cx, int cy)
{
	if (!HasFBO()){
		return FALSE;
	}

	if (cx == 0 || cy == 0){
		// Cannot make Zero size FBO
		return FALSE;
	}

	glBindTexture(m_nTextureUnit, m_nFrameBufferTex);
	glTexImage2D(m_nTextureUnit, 0, GL_RGBA32F, cx, cy, 0, GL_RGBA, GL_FLOAT, NULL);
	bind();

	glTexParameteri(m_nTextureUnit, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(m_nTextureUnit, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(m_nTextureUnit, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(m_nTextureUnit, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_nTextureUnit,
		m_nFrameBufferTex, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthbufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, cx, cy);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_nDepthbufferObject);

	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_bufs);

	unbind();

	if (!HasFBO()) {
		return FALSE;
	}
	return TRUE;
}

BOOL CFrameRenderBuffer::CheckHardware()
{
	// FBO and Rect Texture Check
	//if (glewGetExtension("GL_EXT_framebuffer_object") != GL_TRUE) {
	if (glewIsSupported("GL_EXT_framebuffer_object") != GL_TRUE) {
		TRACE0("Driver does not support Framebuffer Objects (GL_EXT_framebuffer_object)\n");
		return FALSE;
	}
	return TRUE;
}

void CFrameRenderBuffer::GLGetScreenSize(int& nOgnX, int& nOgnY, int& nWidth, int& nHeight)
{
	nWidth = ((m_size.cx - 1) / 4 + 1) * 4;
	nHeight = m_size.cy;

	nWidth = max(4, nWidth);
	nHeight = max(4, nHeight);

	nOgnX = 0;
	nOgnY = 0;
}

bool CFrameRenderBuffer::CheckFrameBuffer()
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != status) {
#ifdef DEBUG
		TRACE0("ERROR: incomplete framebuffer\n");
		if (GL_FRAMEBUFFER_UNDEFINED == status) {
			TRACE0("GL_FRAMEBUFFER_UNDEFINED\n");
		}
		else if (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT == status) {
			TRACE0("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
		}
		else if (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT == status) {
			TRACE0("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
		}
		else if (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER == status) {
			TRACE0("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");
		}
		else if (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER == status) {
			TRACE0("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n");
		}
		else if (GL_FRAMEBUFFER_UNSUPPORTED == status) {
			TRACE0("GL_FRAMEBUFFER_UNSUPPORTED\n");
		}
		else if (GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE == status) {
			TRACE0("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n");
		}
		else if (GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS == status) {
			TRACE0("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n");
		}
		else {
			TRACE0("unspecified error\n");
		}
#endif // DEBUG
		return false;
	}
	return true;
}

void CFrameRenderBuffer::copyDepthBuffer()
{
	int nOgnX, nOgnY, nWidth, nHeight;
	GLGetScreenSize(nOgnX, nOgnY, nWidth, nHeight);

	int nMemBufferSize = nWidth * nHeight * 3 * sizeof(GLubyte);
	if (m_ncolorBufferSize != nMemBufferSize)
	{
		if (m_pColorBuffer != NULL) delete[] m_pColorBuffer;
		m_pColorBuffer = new GLubyte[nMemBufferSize];
		m_ncolorBufferSize = nMemBufferSize;
	}

	glReadPixels(nOgnX, nOgnY, nWidth, nHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, (GLvoid *)m_pColorBuffer);
}

void CFrameRenderBuffer::copyColorBuffer()
{
}
