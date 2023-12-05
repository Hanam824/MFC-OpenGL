#include "pch.h"
#include "GL/glew.h"
#include "FBOManager.h"
#include "framebufferObject.h"
#include "renderbuffer.h"

#include<iostream>
#include<fstream>

#define FBO_RENDER	GL_COLOR_ATTACHMENT0_EXT
#define FBO_OVERLAY	GL_COLOR_ATTACHMENT1_EXT
#define FBO_COLORCODE GL_COLOR_ATTACHMENT2_EXT
#pragma warning( disable : 4996)

namespace
{
	bool SaveImage(const char* szPathName, void* lpBits, int w, int h)
	{
		//Create a new file for writing
		FILE *pFile = fopen(szPathName, "wb");

		if (pFile == NULL) {
			return false;
		}

		BITMAPINFOHEADER BMIH;
		BMIH.biSize = sizeof(BITMAPINFOHEADER);
		BMIH.biSizeImage = w * h * 3;
		// Create the bitmap for this OpenGL context
		BMIH.biSize = sizeof(BITMAPINFOHEADER);
		BMIH.biWidth = w;
		BMIH.biHeight = h;
		BMIH.biPlanes = 1;
		BMIH.biBitCount = 24;
		BMIH.biCompression = BI_RGB;
		BMIH.biSizeImage = w * h * 3;
		BMIH.biXPelsPerMeter = 0;//CID 19539
		BMIH.biYPelsPerMeter = 0;//CID 19539
		BMIH.biClrUsed = 0;//CID 19539
		BMIH.biClrImportant = 0;//CID 19539
		BITMAPFILEHEADER bmfh;
		int nBitsOffset = sizeof(BITMAPFILEHEADER) + BMIH.biSize;
		LONG lImageSize = BMIH.biSizeImage;
		LONG lFileSize = nBitsOffset + lImageSize;
		bmfh.bfType = 'B' + ('M' << 8);
		bmfh.bfOffBits = nBitsOffset;
		bmfh.bfSize = lFileSize;
		bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
		//Write the bitmap file header
		size_t nWrittenFileHeaderSize = fwrite(&bmfh, (size_t)1, sizeof(BITMAPFILEHEADER), pFile);

		//And then the bitmap info header

		size_t nWrittenInfoHeaderSize = fwrite(&BMIH, (size_t)1, sizeof(BITMAPINFOHEADER), pFile);

		//Finally, write the image data itself
		//-- the data represents our drawing
		size_t nWrittenDIBDataSize = fwrite(lpBits, (size_t)1, lImageSize, pFile);
		fclose(pFile);
		return true;
	}

	bool SaveFile(const char* szPathName, GLubyte* lpBits, int w, int h)
	{
		std::ofstream file;
		file.open(szPathName);

		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				int index = (w * y + x) * 3;
				unsigned char r = lpBits[index];
				unsigned char g = lpBits[index + 1];
				unsigned char b = lpBits[index + 2];
				if (r == 0 && g == 0 && b == 0){
					// skip
				}
				else
					file << static_cast<int>(r) << "," << static_cast<int>(g) << "," << static_cast<int>(b) /*<< "," << a*/ << std::endl;
			} 
		}

		file.close();
		return true;
	}
}

CFBOManager::CFBOManager()
{
	m_pFBO = NULL;
	m_pRB   = NULL;
	m_size.cx = 0;
	m_size.cy = 0;

	m_eCurBuffer = NONE;
	m_pMemBuffer = NULL;
	m_nMemBufferSize = 0;
	m_bCapture = false;
}

CFBOManager::~CFBOManager()
{
	if( m_pMemBuffer != NULL )
	{
		delete [] m_pMemBuffer;
	}
	if( m_pFBO )
	{		
		delete m_pFBO;
		m_pFBO = NULL;
	}
	if( m_pRB != NULL )
	{
		delete m_pRB;
		m_pRB = NULL;
	}
}


void CFBOManager::GLProcessFBO( int cx, int cy, bool bUseCapture )
{
	m_bCapture = bUseCapture;
	if( !CheckHardware() )
	{
		GLRemoveFBO();
		return;
	}

	if( HasFBO() )
	{
		if( IsSizeChanged(cx,cy) )
		{
			if( GLResizeFBO(cx,cy) )
			{
				m_size.SetSize(cx,cy);
			}
			else
			{
				GLRemoveFBO();
				m_size.SetSize(0,0);
			}
		}
	}
	else
	{
		if( GLCreateFBO(cx,cy) )
		{
			m_size.SetSize(cx,cy);
		}
		else
		{
			GLRemoveFBO();
			m_size.SetSize(0,0);
		}
	}
}


BOOL CFBOManager::GLCreateFBO(int cx, int cy)
{
	GLuint texID[3];
	glGenTextures(3, texID); // create (reference to) a new texture

	for(int i=0; i<3; i++)
	{
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID[i]);
		// (set texture parameters here)
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		//create the texture
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	if( m_pFBO == NULL ) m_pFBO = new FramebufferObject;
	if( m_pRB == NULL ) m_pRB = new Renderbuffer;

	m_pFBO->Bind();	// Bind framebuffer object.

	// Attach texture to framebuffer color buffer
	m_pFBO->AttachTexture(GL_TEXTURE_RECTANGLE_ARB, texID[0], FBO_RENDER);	
	m_pFBO->AttachTexture(GL_TEXTURE_RECTANGLE_ARB, texID[1], FBO_OVERLAY);	
	m_pFBO->AttachTexture(GL_TEXTURE_RECTANGLE_ARB, texID[2], FBO_COLORCODE);	

	// Optional: initialize depth renderbuffer
 	m_pRB->Set( GL_DEPTH_STENCIL_EXT, cx, cy );
 	m_pFBO->AttachRenderBuffer( m_pRB->GetId(), GL_DEPTH_ATTACHMENT_EXT );
 	m_pFBO->AttachRenderBuffer( m_pRB->GetId(), GL_STENCIL_ATTACHMENT_EXT );

	// Disable FBO rendering for now...
	m_pFBO->Disable();

	// Validate the FBO after attaching textures and render buffers
	if( !m_pFBO->IsValid() )
	{
		return FALSE;
	}	
	return TRUE;
}

void CFBOManager::GLRemoveFBO()
{
	if( m_pFBO )
	{
		UINT texID;
		texID = m_pFBO->GetAttachedId(FBO_RENDER);		
		if( glIsTexture(texID) )
		{
			glDeleteTextures(3,&texID);
		}		
		delete m_pFBO;
		m_pFBO = NULL;
	}

	if( m_pRB != NULL )
	{
		delete m_pRB;
		m_pRB = NULL;
	}
}

BOOL CFBOManager::HasFBO()
{
	return ( m_pFBO != NULL && m_pFBO->IsValid() );
}

BOOL CFBOManager::GLResizeFBO(int cx, int cy)
{
	if( !HasFBO() )
	{
		return FALSE;
	}
	
	if( cx == 0 || cy == 0 )
	{
		// Cannot make Zero size FBO
		return FALSE;
	}

	UINT texID[3];
	texID[0] = m_pFBO->GetAttachedId(FBO_RENDER);
	texID[1] = m_pFBO->GetAttachedId(FBO_OVERLAY);
	texID[2] = m_pFBO->GetAttachedId(FBO_COLORCODE);

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID[0]);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID[1]);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID[2]);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	m_pFBO->Bind(); // Bind framebuffer object.

	// Optional: initialize depth renderbuffer
	m_pRB->Set( GL_DEPTH_STENCIL_EXT, cx, cy );
	m_pFBO->AttachRenderBuffer( m_pRB->GetId(), GL_DEPTH_ATTACHMENT_EXT );
	m_pFBO->AttachRenderBuffer( m_pRB->GetId(), GL_STENCIL_ATTACHMENT_EXT );

	// Disable FBO rendering for now...
	FramebufferObject::Disable();

	// Validate the FBO after attaching textures and render buffers
	if( !m_pFBO->IsValid() ){
		return FALSE;
	}

	return TRUE;
}

BOOL CFBOManager::CheckHardware()
{
	// FBO and Rect Texture Check
	//if (glewGetExtension("GL_EXT_framebuffer_object") != GL_TRUE) {
	if (glewIsSupported("GL_EXT_framebuffer_object") != GL_TRUE) {
		TRACE0("Driver does not support Framebuffer Objects (GL_EXT_framebuffer_object)\n");
		return FALSE;
	}
	return TRUE;
}

BOOL CFBOManager::IsSizeChanged(int cx, int cy)
{
	if( m_size.cx == cx && m_size.cy == cy )
	{
		return FALSE;	
	}
	return TRUE;
}

void CFBOManager::GLBeginFBO( DRAW_BUFFER eDrawBuffer, DRAW_BUFFER eTargetLayer/*=RENDER*/ )
{
	m_eCurBuffer = eDrawBuffer;
	if( m_eCurBuffer == RENDER )
	{
		if( HasFBO() )
		{
			m_pFBO->Bind();
			glDrawBuffer(FBO_RENDER); 
			glReadBuffer(FBO_RENDER); 
		}
		else
		{
			glDrawBuffer(GL_BACK);
			glReadBuffer(GL_BACK); 
		}
	}
	else if( m_eCurBuffer == OVERLAY )
	{
		if( HasFBO() )
		{
			m_pFBO->Bind();
			glDrawBuffer(FBO_OVERLAY);
			glReadBuffer(FBO_OVERLAY);
			GLRestoreFBO(FBO_RENDER);
		}
		else
		{
			glDrawBuffer(GL_BACK);
			glReadBuffer(GL_BACK); 
			GLRestoreBack();
		}
	}
	else if( m_eCurBuffer == COLORCODE )
	{
		if( HasFBO() )
		{
			m_pFBO->Bind();
			glDrawBuffer(FBO_COLORCODE);
			glReadBuffer(FBO_COLORCODE);
		}
		else
		{
			glDrawBuffer(GL_BACK);
			glReadBuffer(GL_BACK); 
		}
	}
	else if( m_eCurBuffer == LASTLAY )
	{
		if( HasFBO() )
		{
			glDrawBuffer(GL_BACK);
			glReadBuffer(GL_BACK);
			if (eTargetLayer == OVERLAY) {
				GLRestoreFBO(FBO_OVERLAY);
			} else {
				GLRestoreFBO(FBO_RENDER);
			}
		}
		else
		{
			glDrawBuffer(GL_BACK);
			glReadBuffer(GL_BACK); 
			GLRestoreBack();
		}
	}
	else
	{
		glDrawBuffer(GL_BACK);
		glReadBuffer(GL_BACK); 
	}
	
}

void CFBOManager::GLEndFBO()
{
	if( m_eCurBuffer == RENDER )
	{
		if( HasFBO() )
		{
			m_pFBO->Disable();
			glDrawBuffer(GL_BACK);	
			glReadBuffer(GL_BACK); 
			GLRestoreFBO(FBO_RENDER);
		}
		else
		{
			glDrawBuffer(GL_BACK);	
			glReadBuffer(GL_BACK);

			if( m_bCapture ) 
			{
				GLCaptureBack();
			}
		}
	}
	else if( m_eCurBuffer == OVERLAY )
	{
		if( HasFBO() )
		{
			m_pFBO->Disable();
			glDrawBuffer(GL_BACK);
			glReadBuffer(GL_BACK);			
			GLRestoreFBO(FBO_OVERLAY);
		}
		else
		{
			// nothing
		}
	}
	else if( m_eCurBuffer == COLORCODE )
	{
		if( HasFBO() )
		{
			m_pFBO->Disable();
			glDrawBuffer(GL_BACK);
			glReadBuffer(GL_BACK);			
			GLRestoreFBO(FBO_COLORCODE);
#ifdef DEBUG
			if (m_bDebugMode) {
				DebugBuffer("D:\\temp");
				m_bDebugMode = FALSE;
			}
#endif // DEBUG
		}
		else
		{
			// nothing
		}
	}
	else if( m_eCurBuffer == LASTLAY )
	{
		// nothing
	}
}

// EndFBO without Restore
void CFBOManager::GLDisableFBO()
{
	if( HasFBO() )
	{
		m_pFBO->Disable();
		glDrawBuffer(GL_BACK);	
		glReadBuffer(GL_BACK); 
	}
	else
	{
		glDrawBuffer(GL_BACK);
		glReadBuffer(GL_BACK);
	}
}

void CFBOManager::GLRestoreFBO(UINT fboBuffer)
{
	if( HasFBO() )
	{
		UINT texID = m_pFBO->GetAttachedId(fboBuffer);
		if( glIsTexture(texID) )
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			{
				glPushMatrix();
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				gluOrtho2D( 0.0f, 1.0f, 0.0f, 1.0f);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();			

				glDisable(GL_DEPTH_TEST);
				glDisable(GL_LIGHTING);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glDisable(GL_CULL_FACE);

				// Texture Background
				glEnable(GL_TEXTURE_RECTANGLE_ARB);
				glBindTexture( GL_TEXTURE_RECTANGLE_ARB, texID);
				glColor3f( 1.f, 1.f, 1.f );
				glBegin(GL_QUADS);		
				glTexCoord2i(0, m_size.cy); glVertex3f( 0.0f,  1.0f, 0.0f);
				glTexCoord2i(m_size.cx, m_size.cy); glVertex3f( 1.0f,  1.0f, 0.0f);
				glTexCoord2i(m_size.cx, 0); glVertex3f( 1.0f,  0.0f, 0.0f);		
				glTexCoord2i(0, 0); glVertex3f( 0.0f,  0.0f, 0.0f);		
				glEnd();		
				glDisable(GL_TEXTURE_RECTANGLE_ARB);

				glEnable(GL_DEPTH_TEST);
				glEnable(GL_LIGHTING);
				glEnable(GL_CULL_FACE);

				glPopMatrix();
				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);	
				glPopMatrix();
			}
			glPopAttrib();
		}
	}
}

void CFBOManager::GLCaptureBack()
{
	int nOgnX, nOgnY, nWidth, nHeight;
	GLGetScreenSize(nOgnX, nOgnY, nWidth, nHeight);

	int nMemBufferSize = nWidth * nHeight * 3 * sizeof(GLubyte);
	if( m_nMemBufferSize != nMemBufferSize )
	{
		if( m_pMemBuffer != NULL ) delete [] m_pMemBuffer;
		m_pMemBuffer = new GLubyte[nMemBufferSize];
		m_nMemBufferSize = nMemBufferSize;
	}

	glReadPixels( nOgnX, nOgnY, nWidth, nHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, (GLvoid *)m_pMemBuffer);
}

void CFBOManager::GLRestoreBack()
{
	GLint aiViewPort[4];
	glGetIntegerv(GL_VIEWPORT,aiViewPort);

	int nOgnX, nOgnY, nWidth, nHeight;
	GLGetScreenSize(nOgnX, nOgnY, nWidth, nHeight);

	int nMemBufferSize = nWidth * nHeight * 3 * sizeof(GLubyte);
	if( m_nMemBufferSize != nMemBufferSize ) return;

	glPushAttrib(GL_ENABLE_BIT|GL_TRANSFORM_BIT|GL_VIEWPORT_BIT|GL_COLOR_BUFFER_BIT);
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();
			gluOrtho2D( aiViewPort[0], aiViewPort[2], aiViewPort[1], aiViewPort[3]);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();	
			{
				glLoadIdentity();
				glDisable(GL_DEPTH_TEST);
				glDepthMask(0);
				glRasterPos2i( nOgnX , nOgnY );
				glDrawPixels( nWidth, nHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_pMemBuffer);
				glDepthMask(1);
				glEnable(GL_DEPTH_TEST);
			}
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
		}
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);	
	}
	glPopAttrib();
}

void CFBOManager::GLGetScreenSize( int& nOgnX, int& nOgnY, int& nWidth, int& nHeight )
{
	nWidth = ( ( m_size.cx - 1 )/4 + 1 )*4;
	nHeight = m_size.cy;

	nWidth = max( 4, nWidth );
	nHeight = max( 4, nHeight );

	nOgnX = 0;
	nOgnY = 0;
}

void CFBOManager::DebugBuffer(const char* strFilepath)
{
	int nOgnX, nOgnY, nWidth, nHeight;
	GLGetScreenSize(nOgnX, nOgnY, nWidth, nHeight);

	int nMemBufferSize = nWidth * nHeight * 3 * sizeof(GLubyte);
	auto pColorBuffer = new GLubyte[nMemBufferSize];

	glReadPixels(nOgnX, nOgnY, nWidth, nHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, (GLvoid *)pColorBuffer);

	auto imageName = std::string(strFilepath) + "\\Debug_COLORCODE_Buffer.png";
	auto fileName = std::string(strFilepath) + "\\Debug_COLORCODE_Buffer.DAT";
	SaveImage(imageName.c_str(), pColorBuffer, nWidth, nHeight);
	SaveFile(fileName.c_str(), pColorBuffer, nWidth, nHeight);
	delete[] pColorBuffer;
}