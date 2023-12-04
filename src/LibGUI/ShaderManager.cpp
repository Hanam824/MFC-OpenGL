#include "pch.h"
#include "ShaderManager.h"
#include <fstream>

std::string CShaderManager::LoadShaderTextFromResource(UINT nIDResource)
{
	HMODULE hModule	= AfxFindResourceHandle( MAKEINTRESOURCE(nIDResource), _T("SHADER"));
	HRSRC   hrsrc	= ::FindResource( hModule, MAKEINTRESOURCE(nIDResource), _T("SHADER") );
	DWORD   dwSize  = ::SizeofResource( hModule, hrsrc );
	HGLOBAL hData	= ::LoadResource( hModule, hrsrc );

	char* cgText = new char[dwSize+1];
	char* lpszStr	= (char*)::LockResource( hData );	
	strncpy_s(cgText,(int)dwSize+1, lpszStr,(int)dwSize);
	cgText[dwSize] = 0;
	::FreeResource( hData );
	std::string shaderText(cgText);
	delete[] cgText;
	return shaderText;
}

std::string CShaderManager::LoadShaderTextFromFile(const wchar_t* shaderPath)
{
    std::ifstream inputFile(shaderPath);
	if (inputFile.fail()) return 0;

    std::istreambuf_iterator<char> begin(inputFile);
    std::istreambuf_iterator<char> end;
    std::string fileData(begin, end);

	return fileData;
}