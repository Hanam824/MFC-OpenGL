#pragma once

#include <string>

class AFX_EXT_CLASS CShaderManager
{
public:
	static std::string LoadShaderTextFromResource(UINT nIDResource);
	static std::string LoadShaderTextFromFile(const wchar_t* shaderPath);
};