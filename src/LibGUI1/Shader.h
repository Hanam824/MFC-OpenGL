#pragma once

#include "GL/glew.h"

#define STRINGIFY(A)  #A

class AFX_EXT_CLASS Shader
{
public:
	Shader();
	virtual ~Shader();

	void Clear();
	bool IsValid();
	void Enable();
	void Disable();
	GLuint GetShaderObject();

	GLuint GetAttribLocation(const GLcharARB* Attrib);
	void EnableVertexAttrib(const GLcharARB* Attrib);
	void EnableVertexAttrib(GLuint nLocation);
	void DisableVertexAttrib(const GLcharARB* Attrib);
	void DisableVertexAttrib(GLuint nLocation);

	bool LoadFromFile(const wchar_t* vertexShaderPath, const wchar_t* fragmentShaderPath, ...);
	bool LoadFromResource(UINT vertexShaderID, UINT fragmentShaderID, ...);
	bool LoadFromString(const char* vertexShaderCode, const char* fragmentShaderCode, ...);
	bool LoadFromStringVFG(const char* vertexShaderCode, const char* fragmentShaderCode, const char* geometryShaderCode, ...);

	bool ValidateShader();

	void OutputShdderLog(int shader);
	void OutputProgramLog(int shader);
	GLint GetUniformLocation(const GLcharARB* uniformName);

	void SetUniform(const GLcharARB* uniformName, int count, float val0, float val1 = 0.0, float val2 = 0.0, float val3 = 0.0);
	void SetUniform(const GLcharARB* uniformName, int size, float array_float[]);
	void SetUniform(const GLcharARB* uniformName, int val);
	void SetUniform(const GLcharARB* uniformName, bool val);

	void printActiveUniforms();// for debug purpose
	void printActiveAttribs();// for debug purpose
	void printShaderInfoLog();
	void printProgrammeInfoLog();

	void bindAttribLocation(GLuint location, const char * name);
	void bindFragDataLocation(GLuint location, const char * name);

	static CString GetGLSLVersion();
	static int GetAvailableMemory();// KB
	static int GetTotalMemory();// KB
	static int GetMemoryUsage();// KB
	static void LogError();// KB
private:

	int CreateFromString(const char* shaderCode, int shaderType);
	GLuint m_shaderObject;
	GLuint m_vertexShaderObject;
	GLuint m_fragmentShaderObject;
	GLuint m_geometryShaderObject;

	bool m_enabled; // for debug
};
