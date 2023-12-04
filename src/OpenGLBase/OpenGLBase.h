// OpenGLBase.h : main header file for the OpenGLBase DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COpenGLBaseApp
// See OpenGLBase.cpp for the implementation of this class
//

class COpenGLBaseApp : public CWinApp
{
public:
	COpenGLBaseApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
