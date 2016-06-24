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
	string shader_source = tools::read_file_contents<string>( filepath );
	shader = glCreateShader( type );
	if( !shader )
	{
		throw runtime_error( "glCreateShader failed" );
	}

	const GLchar *source_ptr = static_cast<const GLchar*>( shader_source.c_str() );
	glShaderSource( shader, 1, &source_ptr, NULL );

	glCompileShader( shader );
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

	if( compiled )
	{
		return;
	}

	GLint info_len = 0;
	glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &info_len );
	if( info_len > 1 )
	{
		auto info_log = make_unique<char[]>( sizeof( char )*info_len );
		string error_msg;

		if( info_log )
		{
			glGetShaderInfoLog( shader, info_len, NULL, &info_log.get()[0] );
			error_msg = "Error compiling shader: " + string( &info_log.get()[0] );
		}
		else
		{
			error_msg = "Error compiling shader and also failed to allocate memory for infolog";
		}
		throw runtime_error( error_msg );
	}

	glDeleteShader( shader );
	shader = 0;
	throw runtime_error( "Compiling shader failed and getting info log failed" );
}


Shader::~Shader()
{
	glDeleteShader( shader );
}

