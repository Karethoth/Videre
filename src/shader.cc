#include <GL/glew.h>
#include "shader.hh"
#include "common_tools.hh"

#include <memory>
#include <string>
#include <iostream>
#include <exception>

#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

using namespace std;



Shader::Shader( GLenum type, const string& filepath ) : type( type ), compiled( 0 )
{
	string shaderSource = tools::read_file_contents<string>( filepath );

	shader = glCreateShader( type );
	if( !shader )
	{
		throw runtime_error( "glCreateShader failed" );
	}

	const GLchar *sourcePtr = (GLchar*)shaderSource.c_str();
	glShaderSource( shader, 1, &sourcePtr, NULL );

	glCompileShader( shader );
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

	if( compiled )
	{
		return;
	}

	GLint infoLen = 0;
	glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLen );
	if( infoLen > 1 )
	{
		char* infoLog = new char[sizeof( char )*infoLen];
		string errorMsg;

		if( infoLog )
		{
			glGetShaderInfoLog( shader, infoLen, NULL, infoLog );
			errorMsg = "Error compiling shader: " + string( infoLog );
			delete[] infoLog;
		}
		else
		{
			errorMsg = "Error compiling shader and also failed to allocate memory for infolog";
		}
		throw runtime_error( errorMsg );
	}

	glDeleteShader( shader );
	shader = 0;
	throw runtime_error( "Compiling shader failed and getting info log failed" );
}


Shader::~Shader()
{
	glDeleteShader( shader );
}
