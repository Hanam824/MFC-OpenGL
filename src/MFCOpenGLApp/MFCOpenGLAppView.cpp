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

// MFCOpenGLAppView.cpp : implementation of the MFCOpenGLAppView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCOpenGLApp.h"
#endif

#include "MFCOpenGLAppDoc.h"
#include "MFCOpenGLAppView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "../LibGUI/FBOManager.h"


// CMFCOpenGLAppView

IMPLEMENT_DYNCREATE(CMFCOpenGLAppView, CMFCViewBase)

BEGIN_MESSAGE_MAP(CMFCOpenGLAppView, CMFCViewBase)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFCOpenGLAppView construction/destruction

CMFCOpenGLAppView::CMFCOpenGLAppView() noexcept
{
	// TODO: add construction code here

	m_pFBOManager = new CFBOManager;
}

CMFCOpenGLAppView::~CMFCOpenGLAppView()
{
	if (m_pFBOManager)
	{
		delete m_pFBOManager;
		m_pFBOManager = nullptr;
	}
}

BOOL CMFCOpenGLAppView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CMFCViewBase::PreCreateWindow(cs);
}

void CMFCOpenGLAppView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

// CMFCOpenGLAppView drawing

void CMFCOpenGLAppView::OnDraw(CDC* /*pDC*/)
{
	CMFCOpenGLAppDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	GLOnDraw(FALSE);
}

void CMFCOpenGLAppView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCOpenGLAppView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCOpenGLAppView diagnostics

#ifdef _DEBUG
void CMFCOpenGLAppView::AssertValid() const
{
	CMFCViewBase::AssertValid();
}

void CMFCOpenGLAppView::Dump(CDumpContext& dc) const
{
	CMFCViewBase::Dump(dc);
}

CMFCOpenGLAppDoc* CMFCOpenGLAppView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCOpenGLAppDoc)));
	return (CMFCOpenGLAppDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCOpenGLAppView message handlers

void CMFCOpenGLAppView::GLOnDraw(BOOL bUpdateDB)
{
	auto nWidth = 300;
	auto nHeight = 600;

	WGLActivate();
	{
		m_pFBOManager->GLProcessFBO(nWidth, nHeight, FALSE);

		GLRenderScene();
		::SwapBuffers(m_hDC);
	}
	WGLDeactivate();
}

void CMFCOpenGLAppView::GLRenderScene()
{
	RenderScene();
}

void CMFCOpenGLAppView::RenderScene()
{
	// Clear the window with current clearing color
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Background
	GLDrawBackground();

	SetLight(); // for Intel GPU 

	glClear(GL_DEPTH_BUFFER_BIT);
}
