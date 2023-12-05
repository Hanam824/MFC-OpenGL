#include "pch.h"

#include "ViewBase.h"

// CViewBase

IMPLEMENT_DYNCREATE(CViewBase, CView)

CViewBase::CViewBase()
{

}

CViewBase::~CViewBase()
{
}

BEGIN_MESSAGE_MAP(CViewBase, CView)
END_MESSAGE_MAP()


// CViewBase drawing

void CViewBase::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// CViewBase diagnostics

#ifdef _DEBUG
void CViewBase::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CViewBase::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CViewBase message handlers

void CViewBase::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}