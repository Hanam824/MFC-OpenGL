//////////////////////////////////////////////////////////////////////////
//
// FBO(Frame Buffer Object) header
//
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <gl/GL.h>

class FramebufferObject;
class Renderbuffer;
class CSharedResourceManager;

class AFX_EXT_CLASS CFBOManager
{
public:
	CFBOManager();
	virtual ~CFBOManager();

	enum DRAW_BUFFER
	{
		NONE,
		RENDER,
		COLORCODE,
		OVERLAY,
		LASTLAY
	};
	
	void GLProcessFBO(int cx, int cy, bool bUseCapture);
	void GLRemoveFBO();

	void GLBeginFBO(DRAW_BUFFER eDrawBuffer, DRAW_BUFFER eTargetLayer=RENDER);
	void GLEndFBO();
	void GLDisableFBO();
	BOOL HasFBO();	
	void EnableDebug() { m_bDebugMode = TRUE; }
protected:
	BOOL GLCreateFBO(int cx, int cy);
	BOOL IsSizeChanged(int cx, int cy);
	BOOL GLResizeFBO(int cx, int cy);
	
	BOOL CheckHardware();

	void GLRestoreFBO(UINT fboBuffer);
	void GLCaptureBack();
	void GLRestoreBack();

	void GLGetScreenSize(int& nOgnX, int& nOgnY, int& nWidth, int& nHeight);
	void DebugBuffer(const char* strFilepath);
protected:

	FramebufferObject*	m_pFBO;	// Frame Buffer Object
	Renderbuffer*		m_pRB;	// The renderbuffer object used for depth + stencil
	CSize				m_size;	// restored screen size

	DRAW_BUFFER			m_eCurBuffer;
	bool				m_bCapture;

	// Image Buffer
	unsigned char* m_pMemBuffer;
	unsigned long  m_nMemBufferSize;
	BOOL m_bDebugMode = FALSE;
};