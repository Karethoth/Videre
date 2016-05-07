#include "shaderProgram.hh"

#include <iostream>
#include <exception>

using namespace std;

 ShaderProgram::ShaderProgram(
	const Shader& vertex_shader,
	const Shader& fragment_shader,
	std::map<std::string, GLuint> attributes )
{
	linked = 0;
	program = glCreateProgram();
	if( !program )
	{
		throw runtime_error( "glCreateProgram failed" );
	}

	// Attach the shaders
	glAttachShader( program, vertex_shader.shader );
	glAttachShader( program, fragment_shader.shader );

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
		GLint info_len = 0;
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &info_len );
		if( info_len > 1 )
		{
			char* info_log = new char[sizeof( char )*info_len];
			std::string error_msg;

			if( info_log )
			{
				glGetProgramInfoLog( program, info_len, NULL, info_log );
				error_msg = "Error linking shader program: " + std::string( info_log );
				delete[] info_log;
			}
			else
			{
				error_msg = "Error linking shader program and also failed to allocate memory for infolog";
			}
			throw runtime_error( error_msg );
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
const GLint ShaderProgram::get_uniform( const std::string& uniform_name ) const
{
	if( !program )
	{
		return 0;
	}

	GLint uniform = -1;

	auto it = uniforms.find( uniform_name );
	if( it != uniforms.end() )
	{
		uniform = it->second;
	}
	else
	{
		uniform = glGetUniformLocation( program, uniform_name.c_str() );
		uniforms[uniform_name] = uniform;
	}

	if( uniform == -1 )
	{
		throw runtime_error( "Error: Tried to get location of shader uniform '"
			+ uniform_name + "', which doesn't exist." );
	}

	return uniform;
}

