#include "pch.h"

#include <fstream>
#include <iostream>
#include <string>

#include "Shader.h"
#include "ShaderManager.h"

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
//#define SHADER_DEBUG

static void bxMsgOutDev(const TCHAR* Fmt, ...)
{
}

Shader::Shader()
	: m_shaderObject(0)
	, m_vertexShaderObject(0)
	, m_fragmentShaderObject(0)
	, m_geometryShaderObject(0)
	, m_enabled(false)
{
}

Shader::~Shader()
{
}

void Shader::Clear()
{
	ASSERT(false == m_enabled);

	if (m_shaderObject > 0)
	{
		glDeleteProgram(m_shaderObject);
		m_shaderObject = 0;
	}
	if (m_vertexShaderObject)
	{
		glDeleteShader(m_vertexShaderObject);
		m_vertexShaderObject = 0;
	}
	if (m_fragmentShaderObject)
	{
		glDeleteShader(m_fragmentShaderObject);
		m_fragmentShaderObject = 0;
	}
	if (m_geometryShaderObject)
	{
		glDeleteShader(m_geometryShaderObject);
		m_geometryShaderObject = 0;
	}
}

bool Shader::IsValid()
{
	return (m_shaderObject > 0);
}

void Shader::Enable()
{
	m_enabled = true;
	glUseProgram(m_shaderObject);
}

void Shader::Disable()
{
	m_enabled = false;
	glUseProgram(0);
}

GLuint Shader::GetShaderObject()
{
	return m_shaderObject;
}

GLuint Shader::GetAttribLocation(const GLcharARB * AttribName)
{
	auto loc = glGetAttribLocation(m_shaderObject, AttribName);
	if (loc < 0) {
		//bxMsgOutDev(_T("Attrib: %s not found.\n"), CString(AttribName));
	}
	return loc;
}

void Shader::EnableVertexAttrib(const GLcharARB * Attrib)
{
	auto loc = GetAttribLocation(Attrib);
	//if (loc >= 0) CID 20562
		glEnableVertexAttribArray(loc);
}

void Shader::EnableVertexAttrib(GLuint nLocation)
{
	//if (nLocation >= 0) CID 19590 
		glEnableVertexAttribArray(nLocation);
}

void Shader::DisableVertexAttrib(const GLcharARB * Attrib)
{
	auto loc = GetAttribLocation(Attrib);
	//if (loc >= 0) //CID 20562
		glDisableVertexAttribArray(loc);
}

void Shader::DisableVertexAttrib(GLuint nLocation)
{
	//if (nLocation >= 0)//CID 19707
		glDisableVertexAttribArray(nLocation);
}

// 3rd argument must be the count of shader attributes
bool Shader::LoadFromFile(const wchar_t* vertexShaderPath, const wchar_t* fragmentShaderPath, ...)
{
	ASSERT(false == m_enabled);

	Clear();

	std::string vertexShaderCode = CShaderManager::LoadShaderTextFromFile(vertexShaderPath);
	m_vertexShaderObject = CreateFromString(vertexShaderCode.c_str(), GL_VERTEX_SHADER);
	if (m_vertexShaderObject == 0) return false;
	std::string fragmentShaderCode = CShaderManager::LoadShaderTextFromFile(fragmentShaderPath);
	m_fragmentShaderObject = CreateFromString(fragmentShaderCode.c_str(), GL_FRAGMENT_SHADER);
	if (m_fragmentShaderObject == 0)
	{
		Clear();
		return false;
	}

	m_shaderObject = glCreateProgram();
	glAttachShader(m_shaderObject, m_vertexShaderObject);
	glAttachShader(m_shaderObject, m_fragmentShaderObject);

	// setting shader attributes
	//SET_SHADER_ATTRIBUTES(fragmentShaderPath);
	va_list attributeList; va_start(attributeList, fragmentShaderPath);
	char* nextArg;
	int argCount = va_arg(attributeList, int);
	for (int i = 0; i < argCount; ++i)
	{
		int index = va_arg(attributeList, int);
		nextArg = va_arg(attributeList, char*);
		glBindAttribLocationARB(m_shaderObject, index, nextArg);
	}
	va_end(attributeList);

	glLinkProgram(m_shaderObject);

	return ValidateShader();
}

bool Shader::LoadFromResource(UINT vertexShaderID, UINT fragmentShaderID, ...)
{
	ASSERT(false == m_enabled);

	Clear();

	std::string vertexShaderCode = CShaderManager::LoadShaderTextFromResource(vertexShaderID);
	m_vertexShaderObject = CreateFromString(vertexShaderCode.c_str(), GL_VERTEX_SHADER);
	if (m_vertexShaderObject == 0) return false;
	std::string fragmentShaderCode = CShaderManager::LoadShaderTextFromResource(fragmentShaderID);
	m_fragmentShaderObject = CreateFromString(fragmentShaderCode.c_str(), GL_FRAGMENT_SHADER);
	if (m_fragmentShaderObject == 0)
	{
		Clear();
		return false;
	}

	m_shaderObject = glCreateProgram();
	glAttachShader(m_shaderObject, m_vertexShaderObject);
	glAttachShader(m_shaderObject, m_fragmentShaderObject);

	// setting shader attributes
	//SET_SHADER_ATTRIBUTES(fragmentShaderID);

	va_list attributeList; va_start(attributeList, fragmentShaderID);
	char* nextArg;
	int argCount = va_arg(attributeList, int);
	for (int i = 0; i < argCount; ++i)
	{
		int index = va_arg(attributeList, int);
		nextArg = va_arg(attributeList, char*);
		glBindAttribLocationARB(m_shaderObject, index, nextArg);
	}
	va_end(attributeList);

	glLinkProgram(m_shaderObject);

	return ValidateShader();
}

bool Shader::LoadFromString(const char* vertexShaderCode, const char* fragmentShaderCode, ...)
{
	ASSERT(false == m_enabled);

	Clear();

	m_vertexShaderObject = CreateFromString(vertexShaderCode, GL_VERTEX_SHADER);
	if (m_vertexShaderObject == 0) return false;
	m_fragmentShaderObject = CreateFromString(fragmentShaderCode, GL_FRAGMENT_SHADER);
	if (m_fragmentShaderObject == 0)
	{
		Clear();
		return false;
	}

	m_shaderObject = glCreateProgram();
	glAttachShader(m_shaderObject, m_vertexShaderObject);
	glAttachShader(m_shaderObject, m_fragmentShaderObject);

	// setting shader attributes
	//SET_SHADER_ATTRIBUTES(fragmentShaderCode);

	va_list attributeList; va_start(attributeList, fragmentShaderCode);
	char* nextArg;
	int argCount = va_arg(attributeList, int);
	for (int i = 0; i < argCount; ++i)
	{
		int index = va_arg(attributeList, int);
		nextArg = va_arg(attributeList, char*);
		glBindAttribLocationARB(m_shaderObject, index, nextArg);
	}
	va_end(attributeList);

	glLinkProgram(m_shaderObject);

	return ValidateShader();
}

bool Shader::LoadFromStringVFG(const char* vertexShaderCode,
	const char* fragmentShaderCode,
	const char* geometryShaderCode,
	...)
{
	ASSERT(false == m_enabled);

	Clear();

	if (!GLEW_ARB_geometry_shader4)
	{
		return false;
	}

	m_vertexShaderObject = CreateFromString(vertexShaderCode, GL_VERTEX_SHADER);
	if (m_vertexShaderObject == 0) return false;

	m_fragmentShaderObject = CreateFromString(fragmentShaderCode, GL_FRAGMENT_SHADER);
	if (m_fragmentShaderObject == 0)
	{
		Clear();
		return false;
	}

	m_geometryShaderObject = CreateFromString(geometryShaderCode, GL_GEOMETRY_SHADER);
	if (m_geometryShaderObject == 0)
	{
		Clear();
		return false;
	}

	m_shaderObject = glCreateProgram();
	glAttachShader(m_shaderObject, m_vertexShaderObject);
	glAttachShader(m_shaderObject, m_fragmentShaderObject);
	glAttachShader(m_shaderObject, m_geometryShaderObject);

	// setting shader attributes
	//SET_SHADER_ATTRIBUTES(fragmentShaderCode);

	va_list attributeList; va_start(attributeList, geometryShaderCode);
	char* nextArg;
	int argCount = va_arg(attributeList, int);
	for (int i = 0; i < argCount; ++i)
	{
		int index = va_arg(attributeList, int);
		nextArg = va_arg(attributeList, char*);
		glBindAttribLocationARB(m_shaderObject, index, nextArg);
	}
	va_end(attributeList);

	glLinkProgram(m_shaderObject);

	return ValidateShader();
}


int Shader::CreateFromString(const char* shaderCode, int shaderType)
{
	ASSERT(false == m_enabled);

	GLuint shaderId = glCreateShader(shaderType);

	glShaderSource(shaderId, 1, &shaderCode, 0);
	glCompileShader(shaderId);
	OutputShdderLog(shaderId);

	return shaderId;
}

void Shader::OutputShdderLog(int shader)
{
	int logSize = 0;
	int length = 0;
	char log[1024];
	TCHAR tlog[2048];
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

	if (logSize > 1)
	{
		std::fill(log, log + 1024, 0);
		std::fill(tlog, tlog + 2048, 0);
		glGetShaderInfoLog(shader, 1024, &length, log);
#ifdef UNICODE
		MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, log, (int)strlen(log), tlog, (sizeof tlog) / 2);
		bxMsgOutDev(tlog);
#else
		strcpy(tlog, log);
#endif
	}
}

void Shader::OutputProgramLog(int shader)
{
	int logSize = 0;
	int length = 0;
	char log[1024];
	TCHAR tlog[2048];

	glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logSize);

	if (logSize > 1)
	{
		std::fill(log, log + 1024, 0);
		std::fill(tlog, tlog + 2048, 0);
		glGetProgramInfoLog(shader, 1024, &length, log);
#ifdef UNICODE
		MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, log, (int)strlen(log), tlog, (sizeof tlog) / 2);
		bxMsgOutDev(tlog);
#else
		strcpy(tlog, log);
#endif
	}
}

GLint Shader::GetUniformLocation(const GLcharARB* uniformName)
{
	ASSERT(true == m_enabled);
	return glGetUniformLocationARB(m_shaderObject, uniformName);
}

bool Shader::ValidateShader()
{
	GLint linked;
	OutputProgramLog(m_shaderObject);
	glGetProgramiv(m_shaderObject, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		bxMsgOutDev(_T("[Error] glLinkProgram failed."));
		Clear();
		return false;
	}
	return true;
}

void Shader::SetUniform(const GLcharARB* uniformName, int count, float val0, float val1, float val2, float val3)
{
	auto loc = GetUniformLocation(uniformName);
	if (loc >= 0 && 1 <= count && count <= 4) {
		if (count == 1)glUniform1f(loc, val0);
		else if (count == 2)glUniform2f(loc, val0, val1);
		else if (count == 3)glUniform3f(loc, val0, val1, val2);
		else if (count == 4)glUniform4f(loc, val0, val1, val2, val3);
	}
	else {
#ifdef SHADER_DEBUG
		bxMsgOutDev(_T("Uniform: %s not found.\n"), CString(uniformName));
#endif // SHADER_DEBUG
	}
}

void Shader::SetUniform(const GLcharARB * uniformName, int size, float array_float[])
{
	auto loc = GetUniformLocation(uniformName);
	if (loc >= 0 && 1 <= size && size <= 4) {
		if (size == 1)glUniform1fv(loc, 1, array_float);
		else if (size == 2)glUniform2fv(loc, 1, array_float);
		else if (size == 3)glUniform3fv(loc, 1, array_float);
		else if (size == 4)glUniform4fv(loc, 1, array_float);
	}
	else {
#ifdef SHADER_DEBUG
		bxMsgOutDev(_T("Uniform: %s not found.\n"), CString(uniformName));
#endif // SHADER_DEBUG
	}
		
}

void Shader::SetUniform(const GLcharARB * uniformName, int val)
{
	auto loc = GetUniformLocation(uniformName);
	if (loc >= 0)
		glUniform1i(loc, val);
}

void Shader::SetUniform(const GLcharARB * uniformName, bool val)
{
	auto loc = GetUniformLocation(uniformName);
	if (loc >= 0)
	{
		if (val == true)
			glUniform1i(loc, 1);
		else
			glUniform1i(loc, 0);
	}
}

void Shader::printActiveUniforms()
{
	if (m_shaderObject == 0)
		bxMsgOutDev(_T("Shader program does not exit!!!!"));
	GLint nUniforms, size, location, maxLen;
	GLchar * name;
	GLsizei written;
	GLenum type;

	glGetProgramiv(m_shaderObject, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
	glGetProgramiv(m_shaderObject, GL_ACTIVE_UNIFORMS, &nUniforms);

	name = (GLchar *)malloc(maxLen);
	bxMsgOutDev(_T(" Location | Name\n"));
	bxMsgOutDev(_T("------------------------------------------------\n"));
	for (int i = 0; i < nUniforms; ++i) {
		glGetActiveUniform(m_shaderObject, i, maxLen, &written, &size, &type, name);
		location = glGetUniformLocation(m_shaderObject, name);
		CString strLoc;
		strLoc.Format(_T(" %-8d | %s\n"), location, name);
		bxMsgOutDev(strLoc);
	}
	free(name);
}

void Shader::printActiveAttribs()
{
	if (m_shaderObject == 0)
		bxMsgOutDev(_T("Shader program does not exit!!!!"));

	GLint written, size, location, maxLength, nAttribs;
	GLenum type;
	GLchar * name;

	glGetProgramiv(m_shaderObject, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
	glGetProgramiv(m_shaderObject, GL_ACTIVE_ATTRIBUTES, &nAttribs);

	name = (GLchar *)malloc(maxLength);

	bxMsgOutDev(_T(" Index | Name\n"));
	bxMsgOutDev(_T("------------------------------------------------\n"));
	for (int i = 0; i < nAttribs; i++) {
		glGetActiveAttrib(m_shaderObject, i, maxLength, &written, &size, &type, name);
		location = glGetAttribLocation(m_shaderObject, name);
		CString strLoc;
		strLoc.Format(_T(" %-5d | %s\n"), location, name);
		bxMsgOutDev(strLoc);
	}
	free(name);
}

void Shader::printShaderInfoLog()
{
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog(m_shaderObject, max_length, &actual_length, log);
	bxMsgOutDev(_T("shader info log for GL index %i:\n%s\n"), m_shaderObject, log);
}

void Shader::printProgrammeInfoLog()
{
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog(m_shaderObject, max_length, &actual_length, log);
	bxMsgOutDev(_T("program info log for GL index %u:\n%s"), m_shaderObject, log);

}

void Shader::bindAttribLocation(GLuint location, const char * name)
{
	glBindAttribLocation(m_shaderObject, location, name);
}

void Shader::bindFragDataLocation(GLuint location, const char * name)
{
	glBindFragDataLocation(m_shaderObject, location, name);
}

CString Shader::GetGLSLVersion()
{
	auto shaderVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	CString strVer;
	if (shaderVersion)
		strVer.Format(_T("%s"), CString(shaderVersion));
	auto pos = strVer.Find(_T(" "));
	return strVer.Mid(0, pos);
}

int Shader::GetAvailableMemory()
{
	GLint cur_avail_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
		&cur_avail_mem_kb);
	return cur_avail_mem_kb;
}

int Shader::GetTotalMemory()
{
	GLint total_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
		&total_mem_kb);
	return total_mem_kb;
}

int Shader::GetMemoryUsage()
{
	GLint cur_avail_mem_kb = 0;
	GLint total_mem_kb = 0;

	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
		&cur_avail_mem_kb);
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
		&total_mem_kb);

	return (total_mem_kb - cur_avail_mem_kb);
}

void Shader::LogError()
{
	auto error = glGetError();
	switch (error)
	{
	case GL_NO_ERROR:
		bxMsgOutDev(_T("No error has been recorded\n"));
		break;
	case GL_INVALID_ENUM:
		bxMsgOutDev(_T("An unacceptable value is specified for an enumerated argument\n"));
		break;
	case GL_INVALID_VALUE:
		bxMsgOutDev(_T("A numeric argument is out of range\n"));
		break;
	case GL_INVALID_OPERATION:
		bxMsgOutDev(_T("The specified operation is not allowed in the current state\n"));
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		bxMsgOutDev(_T("The framebuffer object is not complete\n"));
		break;
	case GL_OUT_OF_MEMORY:
		bxMsgOutDev(_T("There is not enough memory left to execute the command\n"));
		break;
	case GL_STACK_UNDERFLOW:
		bxMsgOutDev(_T("An attempt has been made to perform an operation that would cause an internal stack to underflow.\n"));
		break;
	case GL_STACK_OVERFLOW:
		bxMsgOutDev(_T("An attempt has been made to perform an operation that would cause an internal stack to overflow.\n"));
		break;
	default:
		break;
	}
}
