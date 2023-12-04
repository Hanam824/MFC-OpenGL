// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// MFCOpenGLAppView.h : interface of the CMFCOpenGLAppView class
//

#pragma once
#include "../MFCAppBase/MFCViewBase.h"


class CFBOManager;

class CMFCOpenGLAppView : public CMFCViewBase
{
protected: // create from serialization only
	CMFCOpenGLAppView() noexcept;
	DECLARE_DYNCREATE(CMFCOpenGLAppView)

// Attributes
public:
	CMFCOpenGLAppDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

// Implementation
public:
	virtual ~CMFCOpenGLAppView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()


// Operations
public:
	void GLOnDraw(BOOL bUpdateDB = FALSE);

	void GLRenderScene();

	void RenderScene();

private:
	CFBOManager* m_pFBOManager;
};

#ifndef _DEBUG  // debug version in MFCOpenGLAppView.cpp
inline CMFCOpenGLAppDoc* CMFCOpenGLAppView::GetDocument() const
   { return reinterpret_cast<CMFCOpenGLAppDoc*>(m_pDocument); }
#endif

