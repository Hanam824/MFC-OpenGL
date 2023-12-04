#pragma once

#include "ViewBase.h"

#include "gl/glew.h"
#include "gl/wglew.h"


class AFX_EXT_CLASS CGLCtrlView : public CViewBase
{
	DECLARE_DYNCREATE(CGLCtrlView)

public:
	CGLCtrlView();
	virtual ~CGLCtrlView();

	// Overrides
public:
	virtual void OnInitialUpdate();

	// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd*);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

public:
	void WGLActivate();
	void WGLDeactivate();
	HDC GetHDC() { return m_hDC; };
	HGLRC GetHRC() { return m_hRC; };

protected:
	void GLSetupRC(void*);
	void InitializePalette(void);
	void SetLight();
	//void CreateSmallTextLists(HDC hDC);
	//void CreateTextLists(HDC hDC);
	//BOOL GetGLInfo();
	void ShareGLResource();

	void SetupGLContextExt();

	void GLDrawBackground();

protected:
	HGLRC	m_hRC;	// RC Handler
	HDC		m_hDC;	// DC Handler
	CPalette m_GLPalette;	// Logical Palette

	COLORREF	m_colBack1;
	COLORREF	m_colBack2;
	int			m_BackGroundAlpha;

private:

};