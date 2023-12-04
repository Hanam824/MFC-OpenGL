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


// CMFCOpenGLAppView

IMPLEMENT_DYNCREATE(CMFCOpenGLAppView, CView)

BEGIN_MESSAGE_MAP(CMFCOpenGLAppView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFCOpenGLAppView construction/destruction

CMFCOpenGLAppView::CMFCOpenGLAppView() noexcept
{
	// TODO: add construction code here

}

CMFCOpenGLAppView::~CMFCOpenGLAppView()
{
}

BOOL CMFCOpenGLAppView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMFCOpenGLAppView drawing

void CMFCOpenGLAppView::OnDraw(CDC* /*pDC*/)
{
	CMFCOpenGLAppDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
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
	CView::AssertValid();
}

void CMFCOpenGLAppView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCOpenGLAppDoc* CMFCOpenGLAppView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCOpenGLAppDoc)));
	return (CMFCOpenGLAppDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCOpenGLAppView message handlers
