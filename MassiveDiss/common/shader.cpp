#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "shader.h"

GLuint prepareShader(const char* path, GLenum shaderType)
{
	// create shader
	GLuint shaderID = glCreateShader(shaderType);

	// read in code from file
	std::string shaderCode;
	std::ifstream shaderStream(path, std::ios::in);
	if (shaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << shaderStream.rdbuf();
		shaderCode = sstr.str();
		shaderStream.close();
	}
	else
	{
		printf("Can't open shader %s!\n", path);
		getchar();
		return 0;
	}

	// compile shader
	printf("Compiling shader: %s\n", path);
	char const * sourcePointer = shaderCode.c_str();
	glShaderSource(shaderID, 1, &sourcePointer, NULL);
	glCompileShader(shaderID);

	GLint result = GL_FALSE;
	int infoLogLength;

	// check shader
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0)
	{
		std::vector<char> errorMessage(infoLogLength + 1);
		glGetShaderInfoLog(shaderID, infoLogLength, NULL, &errorMessage[0]);
		printf("Error message: %s\n", &errorMessage[0]);
		getchar();
	}
	
	return shaderID;
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) 
{
	// create the shaders
	GLuint VertexShaderID = prepareShader(vertex_file_path, GL_VERTEX_SHADER);
	GLuint FragmentShaderID = prepareShader(fragment_file_path, GL_FRAGMENT_SHADER);

	// link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	GLint result = GL_FALSE;
	int infoLogLength;

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> errorMessage(infoLogLength + 1);
		glGetProgramInfoLog(ProgramID, infoLogLength, NULL, &errorMessage[0]);
		printf("Error message: %s\n", &errorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
