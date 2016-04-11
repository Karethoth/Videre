#include "shaderProgram.hh"

#include <iostream>
#include <exception>

using namespace std;

 ShaderProgram::ShaderProgram(
	const Shader& vertexShader,
	const Shader& fragmentShader,
	std::map<std::string, GLuint> attributes )
{
	linked = 0;
	program = glCreateProgram();
	if( !program )
	{
		throw runtime_error( "glCreateProgram failed" );
	}

	// Attach the shaders
	glAttachShader( program, vertexShader.shader );
	glAttachShader( program, fragmentShader.shader );

	// Bind attribute locations as requested
	for( auto& attr : attributes )
	{
		glBindAttribLocation( program, attr.second, attr.first.c_str() );
	}

	// Link the shader program
	glLinkProgram( program );
	glGetProgramiv( program, GL_LINK_STATUS, &linked );
	if( !linked )
	{
		GLint infoLen = 0;
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &infoLen );
		if( infoLen > 1 )
		{
			char* infoLog = new char[sizeof( char )*infoLen];
			std::string errorMsg;

			if( infoLog )
			{
				glGetProgramInfoLog( program, infoLen, NULL, infoLog );
				errorMsg = "Error linking shader program: " + std::string( infoLog );
				delete[] infoLog;
			}
			else
			{
				errorMsg = "Error linking shader program and also failed to allocate memory for infolog";
			}
			throw runtime_error( errorMsg );
		}

		glDeleteProgram( program );
		throw runtime_error( "Error linking shader program and failed to get infolog " );
	}

	// Check out how the attributes were bound actually
	for( auto& attr : attributes )
	{
		this->attributes[attr.first] = glGetAttribLocation( program, attr.first.c_str() );
	}
}


ShaderProgram::~ShaderProgram()
{
	wcout << "Deleting program " << program << endl;
	if( program )
	{
		glDeleteProgram( program );
	}
}



// Fetches the requested uniform location
const GLint ShaderProgram::GetUniform( const std::string& uniformName ) const
{
	if( !program )
	{
		return 0;
	}

	GLint uniform = -1;

	auto it = uniforms.find( uniformName );
	if( it != uniforms.end() )
	{
		uniform = it->second;
	}
	else
	{
		uniform = glGetUniformLocation( program, uniformName.c_str() );
		uniforms[uniformName] = uniform;
	}

	if( uniform == -1 )
	{
		throw runtime_error( "Error: Tried to get location of shader uniform '"
			+ uniformName + "', which doesn't exist." );
	}

	return uniform;
}

