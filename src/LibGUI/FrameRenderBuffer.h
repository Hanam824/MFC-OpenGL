//////////////////////////////////////////////////////////////////////////
//
// FBO(Frame Buffer Object) header
//
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <gl/GL.h>

class Shader;

enum class CFrameBufferType
{
	FR_COLOR_BUFFER = 0,
	FR_DEPTH_BUFFER = 1,
	FR_STENCIL_BUFFER = 2,
	FR_DEPTH_STENCIL_BUFFER = 3,
};

class AFX_EXT_CLASS CFrameRenderBuffer
{
public:
	CFrameRenderBuffer();
	virtual ~CFrameRenderBuffer();

	BOOL GLCreateFBO(int cx, int cy);

	bool GLProcessFBO(int cx, int cy, bool bUseCapture = false);
	bool IsInitialized() { return m_bInitial?true:false;}
	void clear();
	void GLBeginFBO();
	void GLEndFBO();
	void bind();
	void unbind();

	BOOL HasFBO();
	void GLRemoveFBO();

	GLuint GetTextureID() { return m_nFrameBufferTex; }
	void DrawTexture(GLuint texId);
	void GetPixel(int posx, int posy, float pData[4]);
	void SetRender(BOOL bRender = TRUE) { m_bRender = bRender; }
protected:
	// color Buffer
	unsigned char* m_pColorBuffer = nullptr;
	unsigned long  m_ncolorBufferSize = 0;
	// depth buffer
	float* m_pDepthBuffer = nullptr;
	float* m_pStencilBuffer = nullptr;

	BOOL m_bRender = FALSE;
	BOOL m_bInitial = FALSE;
	CFrameBufferType m_FrameBufferType = CFrameBufferType::FR_COLOR_BUFFER;

	GLuint m_nFrameBufferObject = 0;// frame buffer
	GLuint m_nDepthbufferObject = 0; // depth buffer
	GLuint m_nStencilbufferObject = 0; // Stencil buffer
	GLuint m_nFrameBufferTex = 0;// frame texture

	GLuint m_nScreenVAO = 0;// render screen VAO
	GLint m_ncurrentFB = 0;

	GLenum  m_nTextureUnit = GL_TEXTURE_2D;
	BOOL  m_bCapture = FALSE;
	CSize m_size;	// restored screen size
	std::unique_ptr<Shader> m_pTextureShader = nullptr;
private:
	void GLRestoreFBO();
	void GLGetScreenSize(int& nOgnX, int& nOgnY, int& nWidth, int& nHeight);
	bool CheckFrameBuffer();
	void copyDepthBuffer();
	void copyColorBuffer();

	BOOL IsSizeChanged(int cx, int cy);
	BOOL GLResizeFBO(int cx, int cy);
	BOOL CheckHardware();
};
