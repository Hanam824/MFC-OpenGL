// CViewBase view

class AFX_EXT_CLASS CViewBase : public CView
{
	DECLARE_DYNCREATE(CViewBase)

protected:
	CViewBase();           // protected constructor used by dynamic creation
	virtual ~CViewBase();

protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()

public:
};
