#include "pch.h"

#include "GLCtrlView.h"

IMPLEMENT_DYNCREATE(CGLCtrlView, CViewBase)

CGLCtrlView::CGLCtrlView()
{
	m_hDC = NULL;
	m_hRC = NULL;

	// Color
	m_colBack1 = RGB(102, 102, 153);
	m_colBack2 = RGB(0, 0, 0);

	m_BackGroundAlpha = 255;
}

CGLCtrlView::~CGLCtrlView()
{
}

BEGIN_MESSAGE_MAP(CGLCtrlView, CViewBase)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CGLCtrlView::OnInitialUpdate()
{
	CViewBase::OnInitialUpdate();
}

int CGLCtrlView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here
	int nPixelFormat;					// Pixel format index
	m_hDC = ::GetDC(m_hWnd);			// Get the Device context

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// Size of this structure
		1,								// Version of this structure	
		PFD_DRAW_TO_WINDOW |			// Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |			// Support OpenGL calls in window
		PFD_SWAP_EXCHANGE |
		PFD_DOUBLEBUFFER,				// Double buffered mode
		PFD_TYPE_RGBA,					// RGBA Color mode
		24,								// Want 24bit color 
		0,0,0,0,0,0,					// Not used to select mode
		0,0,							// Not used to select mode
		0,0,0,0,0,						// Not used to select mode
		32,								// Size of depth buffer
		1,								// Not used to select mode
		0,								// Not used to select mode
		PFD_MAIN_PLANE,					// Draw in main plane
		0,								// Not used to select mode
		0,0,0 };						// Not used to select mode

	// Choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);

	// Set the pixel format for the device context
	VERIFY(SetPixelFormat(m_hDC, nPixelFormat, &pfd));

	// Create the rendering context
	m_hRC = wglCreateContext(m_hDC);

	// share gl resources (e.g. vbo) between views
	ShareGLResource();

	// Make the rendering context current, perform initialization, then
	// deselect it
	WGLActivate();
	GLSetupRC(m_hDC);
	glewInit();
	SetupGLContextExt();
	WGLDeactivate();

	// Create the palette if needed
	InitializePalette();

	WGLActivate();
	//CreateTextLists(m_hDC);
	//CreateSmallTextLists(m_hDC);

	//if(m_pDrawTexText)
	//	m_pDrawTexText->CreateTextObject();

	WGLDeactivate();

	//GetGLInfo();

	return 0;
}

void CGLCtrlView::OnDestroy()
{
	CViewBase::OnDestroy();

	// Clean up rendering context stuff
	wglDeleteContext(m_hRC);
	::ReleaseDC(m_hWnd, m_hDC);
}

BOOL CGLCtrlView::OnQueryNewPalette()
{
	// If the palette was created.
	if ((HPALETTE)m_GLPalette)
	{
		int nRet;

		// Selects the palette into the current device context
		SelectPalette(m_hDC, (HPALETTE)m_GLPalette, FALSE);

		// Map entries from the currently selected palette to
		// the system palette.  The return value is the number 
		// of palette entries modified.
		nRet = RealizePalette(m_hDC);

		// Repaint, forces remap of palette in current window
		InvalidateRect(NULL, FALSE);

		return nRet;
	}

	return CViewBase::OnQueryNewPalette();
}

void CGLCtrlView::OnPaletteChanged(CWnd* pFocusWnd)
{
	if (((HPALETTE)m_GLPalette != NULL) && (pFocusWnd != this))
	{
		// Select the palette into the device context
		SelectPalette(m_hDC, (HPALETTE)m_GLPalette, FALSE);

		// Map entries to system palette
		RealizePalette(m_hDC);

		// Remap the current colors to the newly realized palette
		UpdateColors(m_hDC);
		return;
	}

	CViewBase::OnPaletteChanged(pFocusWnd);
}

void CGLCtrlView::GLSetupRC(void* pData)
{
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	glPolygonOffset(1.0f, 1.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	//glDepthFunc(GL_LESS); // #6841 Parts' display order in Z direction by internal ID of Part
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SetLight();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

// Initializes the CPalette object
void CGLCtrlView::InitializePalette(void)
{
	PIXELFORMATDESCRIPTOR pfd;	// Pixel Format Descriptor
	LOGPALETTE* pPal;			// Pointer to memory for logical palette
	int nPixelFormat;			// Pixel format index
	int nColors;				// Number of entries in palette
	int i;						// Counting variable
	BYTE RedRange, GreenRange, BlueRange;
	// Range for each color entry (7,7,and 3)

	// Get the pixel format index and retrieve the pixel format description
	nPixelFormat = GetPixelFormat(m_hDC);
	DescribePixelFormat(m_hDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	// Does this pixel format require a palette?  If not, do not create a
	// palette and just return NULL
	if (!(pfd.dwFlags & PFD_NEED_PALETTE))
		return;

	// Number of entries in palette.  8 bits yeilds 256 entries
	nColors = 1 << pfd.cColorBits;

	// Allocate space for a logical palette structure plus all the palette entries
	pPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + nColors * sizeof(PALETTEENTRY));

	// Fill in palette header 
	pPal->palVersion = 0x300;		// Windows 3.0
	pPal->palNumEntries = nColors; // table size

	// Build mask of all 1's.  This creates a number represented by having
	// the low order x bits set, where x = pfd.cRedBits, pfd.cGreenBits, and
	// pfd.cBlueBits.  
	RedRange = (1 << pfd.cRedBits) - 1;
	GreenRange = (1 << pfd.cGreenBits) - 1;
	BlueRange = (1 << pfd.cBlueBits) - 1;

	// Loop through all the palette entries
	for (i = 0; i < nColors; i++)
	{
		// Fill in the 8-bit equivalents for each component
		pPal->palPalEntry[i].peRed = (i >> pfd.cRedShift) & RedRange;
		pPal->palPalEntry[i].peRed = (unsigned char)(
			(double)pPal->palPalEntry[i].peRed * 255.0 / RedRange);

		pPal->palPalEntry[i].peGreen = (i >> pfd.cGreenShift) & GreenRange;
		pPal->palPalEntry[i].peGreen = (unsigned char)(
			(double)pPal->palPalEntry[i].peGreen * 255.0 / GreenRange);

		pPal->palPalEntry[i].peBlue = (i >> pfd.cBlueShift) & BlueRange;
		pPal->palPalEntry[i].peBlue = (unsigned char)(
			(double)pPal->palPalEntry[i].peBlue * 255.0 / BlueRange);

		pPal->palPalEntry[i].peFlags = (unsigned char)NULL;
	}

	// Create the palette
	m_GLPalette.CreatePalette(pPal);

	// Go ahead and select and realize the palette for this device context
	SelectPalette(m_hDC, (HPALETTE)m_GLPalette, FALSE);
	RealizePalette(m_hDC);

	// Free the memory used for the logical palette structure
	free(pPal);
}

void CGLCtrlView::SetLight()
{
	//GLfloat afLightPos[] = {1.f, 1.f, 1.f, 0.f};
	GLfloat afLightPos[] = { 0.f, 0.f, 1.f, 0.f };
	GLfloat afLightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	//GLfloat afLightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat afLightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat afLightSpecular[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	//GLfloat afMatSpecular[] =  { 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat afMatSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat afMatDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.f };

	// Enable lighting
	glEnable(GL_LIGHTING);

	// Setup and enable light 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, afLightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, afLightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, afLightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, afLightPos);

	glEnable(GL_LIGHT0);

	// Enable color tracking
	glEnable(GL_COLOR_MATERIAL);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, afMatDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, afMatSpecular);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 64);

	// the following setting decreases the rendering performance
	//glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

BOOL CGLCtrlView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

//void CGLCtrlView::CreateTextLists(HDC hDC)
//{
//	HFONT   font;
//	HFONT	oldfont;									// Used For Good House Keeping
//
//	m_TextList = glGenLists(96);								// Storage For 96 Characters
//
//	font = CreateFont(-1 * 14,							// Height Of Font
//		0,								// Width Of Font
//		0,								// Angle Of Escapement
//		0,								// Orientation Angle
//		FW_BOLD,						// Font Weight
//		FALSE,							// Italic
//		FALSE,							// Underline
//		FALSE,							// Strikeout
//		DEFAULT_CHARSET,					// Character Set Identifier
//		OUT_TT_PRECIS,					// Output Precision
//		CLIP_DEFAULT_PRECIS,			// Clipping Precision
//		ANTIALIASED_QUALITY,			// Output Quality
//		FF_DONTCARE | DEFAULT_PITCH,		// Family And Pitch
//		_T("Tahoma"));					// Font Name
//
//	oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
//	wglUseFontBitmaps(hDC, 32, 96, m_TextList);	// Builds 96 Characters Starting At Character 32
//	m_TextList -= 32;
//	SelectObject(hDC, oldfont);							// Selects The Font We Want
//	DeleteObject(font);
//}
//
//void CGLCtrlView::CreateSmallTextLists(HDC hDC)
//{
//	HFONT   font;
//	HFONT	oldfont;									// Used For Good House Keeping
//
//	m_SmallTextList = glGenLists(96);								// Storage For 96 Characters
//
//	font = CreateFont(-1 * 12,			// Height Of Font
//		0,								// Width Of Font
//		0,								// Angle Of Escapement
//		0,								// Orientation Angle
//		FW_NORMAL,						// Font Weight
//		FALSE,							// Italic
//		FALSE,							// Underline
//		FALSE,							// Strikeout
//		DEFAULT_CHARSET,					// Character Set Identifier
//		OUT_TT_PRECIS,					// Output Precision
//		CLIP_DEFAULT_PRECIS,			// Clipping Precision
//		ANTIALIASED_QUALITY,			// Output Quality
//		FF_DONTCARE | DEFAULT_PITCH,		// Family And Pitch
//		_T("Arial"));					// Font Name
//
//	oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
//	wglUseFontBitmaps(hDC, 32, 96, m_SmallTextList);	// Builds 96 Characters Starting At Character 32
//	m_SmallTextList -= 32;
//	SelectObject(hDC, oldfont);							// Selects The Font We Want
//	DeleteObject(font);
//}

//BOOL CGLCtrlView::GetGLInfo()
//{
//	WGLActivate();
//
//	char* str = 0;
//	char* tok = 0;
//
//	// get vendor string
//	str = (char*)glGetString(GL_VENDOR);
//	if (str)
//		m_myGLInfo.vendor = str;
//	else
//		return FALSE;
//
//	// get renderer string
//
//	// get version string
//	str = NULL;
//	str = (char*)glGetString(GL_VERSION);
//	if (str)
//		m_myGLInfo.version = str;
//	else
//		return FALSE;
//
//	// get all extensions as a string
//	/*str = NULL;
//	str = (char*)glGetString(GL_EXTENSIONS);
//
//	// split extensions
//	if(str)
//	{
//		char sep[] = " ";
//		// Token에 사퓖E풔?String은 반드시 복사후 사퓖E!! - Frovi -
//		char* strBuf = new char[strlen(str) + 10];
//		strcpy( strBuf, str);
//
//		tok = strtok((char*)strBuf, sep);
//		while(tok)
//		{
//			m_myGLInfo.extensions.push_back(tok);    // put a extension into struct
//			tok = strtok(0, " ");               // next token
//		}
//
//		if(strBuf)
//		{
//			delete [] strBuf;
//		}
//		strBuf = NULL;
//	}
//	else
//	{
//		return FALSE;
//	}*/
//
//	// get number of color bits
//	glGetIntegerv(GL_RED_BITS, &m_myGLInfo.redBits);
//	glGetIntegerv(GL_GREEN_BITS, &m_myGLInfo.greenBits);
//	glGetIntegerv(GL_BLUE_BITS, &m_myGLInfo.blueBits);
//	glGetIntegerv(GL_ALPHA_BITS, &m_myGLInfo.alphaBits);
//
//	// get depth bits
//	glGetIntegerv(GL_DEPTH_BITS, &m_myGLInfo.depthBits);
//
//	// get stecil bits
//	glGetIntegerv(GL_STENCIL_BITS, &m_myGLInfo.stencilBits);
//
//	// get max number of lights allowed
//	glGetIntegerv(GL_MAX_LIGHTS, &m_myGLInfo.maxLights);
//
//	// get max texture resolution
//	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_myGLInfo.maxTextureSize);
//
//	// get max number of clipping planes
//	glGetIntegerv(GL_MAX_CLIP_PLANES, &m_myGLInfo.maxClipPlanes);
//
//	// get max modelview and projection matrix stacks
//	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &m_myGLInfo.maxModelViewStacks);
//	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &m_myGLInfo.maxProjectionStacks);
//	glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &m_myGLInfo.maxAttribStacks);
//
//	// get max texture stacks
//	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &m_myGLInfo.maxTextureStacks);
//
//
//	WGLDeactivate();
//
//	return TRUE;
//}

//BOOL CGLCtrlView::IsGLExtensionSupported(const std::string& ext)
//{
//	std::vector<std::string>::const_iterator iter = m_myGLInfo.extensions.begin();
//	std::vector<std::string>::const_iterator endIter = m_myGLInfo.extensions.end();
//
//	while (iter != endIter)
//	{
//		if (ext.compare(*iter) == 0)
//			return TRUE;
//		else
//			++iter;
//	}
//	return FALSE;
//}


void CGLCtrlView::WGLActivate()
{
	VERIFY(wglMakeCurrent(m_hDC, m_hRC));
}

void CGLCtrlView::WGLDeactivate()
{
	VERIFY(wglMakeCurrent(NULL, NULL));
}

void CGLCtrlView::ShareGLResource()
{
	POSITION pos = AfxGetApp()->GetFirstDocTemplatePosition();
	while (pos)
	{
		auto docTempl = AfxGetApp()->GetNextDocTemplate(pos);
		POSITION docPos = docTempl->GetFirstDocPosition();
		while (docPos)
		{
			auto pDoc = docTempl->GetNextDoc(docPos);
			POSITION pos0 = pDoc->GetFirstViewPosition();
			while (pos0)
			{
				auto pView = dynamic_cast<CGLCtrlView*>(pDoc->GetNextView(pos0));
				if (pView && pView != this)
				{
					wglShareLists(pView->m_hRC, m_hRC);
					break;
				}
			}
		}
	}
}

void CGLCtrlView::SetupGLContextExt()
{
	if (wglCreateContextAttribsARB)
	{
		GLint attribs[] =
		{
			// Here we ask for OpenGL 2.1
			WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
			WGL_CONTEXT_MINOR_VERSION_ARB, 1,
			// Uncomment this for forward compatibility mode
#ifdef GL_DEBUG
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#else
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
			// Uncomment this for Compatibility profile
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			// We are using Core profile here
			//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};
		HGLRC compHRC = wglCreateContextAttribsARB(m_hDC, 0, attribs);
		if (compHRC && wglMakeCurrent(m_hDC, compHRC))
		{
			wglDeleteContext(m_hRC);
			m_hRC = compHRC;
#ifdef GL_DEBUG
			if (glDebugMessageCallback)
			{
				glDebugMessageCallback((GLDEBUGPROC)simple_print_callback, NULL);
			}
#endif
		}
	}
}

void CGLCtrlView::GLDrawBackground()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	GLubyte colBack1[4] = { GetRValue(m_colBack1),GetGValue(m_colBack1),GetBValue(m_colBack1), (GLubyte)m_BackGroundAlpha };
	GLubyte colBack2[4] = { GetRValue(m_colBack2),GetGValue(m_colBack2),GetBValue(m_colBack2), (GLubyte)m_BackGroundAlpha };
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0f, 1.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);

	glBegin(GL_QUADS);
	//glColor3f( 0.8f, 0.8f, 1.0f );
	glColor4ubv(colBack2);
	glVertex3f(1.0f, 0.0f, 0.0f);	// v5
	glVertex3f(0.0f, 0.0f, 0.0f);	// v2

	glColor4ubv(colBack1);
	glVertex3f(0.0f, 1.0f, 0.0f); // v1
	glVertex3f(1.0f, 1.0f, 0.0f);	// v
	glEnd();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}