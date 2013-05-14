#include "Program.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#define ERROR_BUFFER_LENGTH 1024

Shader::Shader(GLenum type, const std::string& filename)
    : id(-1)
{
    // Create input file stream
    ifstream file_in(filename.c_str());
    if (!file_in) {
        cerr << "Failed to open shader file " << filename << endl;
        return;
    }
    
    // Create string stream
    stringstream file_string;
    file_string << file_in.rdbuf();
    string source_string = file_string.str();
    const GLchar *source = source_string.c_str();
    
    // Create (compiles) shader
    id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    
    // Check compilation result
    GLint result = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result != GL_TRUE)
    {
        // Buffer for any compilation error messages
        char error[ERROR_BUFFER_LENGTH];
        GLsizei length = 0;
        glGetShaderInfoLog(id, ERROR_BUFFER_LENGTH - 1, &length, error);
        
        // Print error
        cerr << "Shader failed to compile: " << filename << endl;
        cerr << error << endl;
        glDeleteShader(id);
        id = -1;
        return;
    }
}

Program::Program(const Shader& vertexShader, const Shader& fragmentShader)
    : id(glCreateProgram())
{
    if (!vertexShader.Valid() || !fragmentShader.Valid()
        || !AttachShader(vertexShader) || !AttachShader(fragmentShader))
        id = -1;
}

Program::Program(const std::string& vertexShaderFilename,
	const std::string& fragmentShaderFilename)
    : id(glCreateProgram())
{
	Shader vertexShader(GL_VERTEX_SHADER, vertexShaderFilename);
	Shader fragmentShader(GL_FRAGMENT_SHADER, fragmentShaderFilename);
    if (!vertexShader.Valid() || !fragmentShader.Valid()
        || !AttachShader(vertexShader) || !AttachShader(fragmentShader))
        id = -1;
}


bool Program::AttachShader(const Shader& shader)
{
    // Attach and link shader
    glAttachShader(id, shader.GetID());
    glLinkProgram(id);
    
    // Check linking result
    GLint result = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        // Buffer for any linking error messages
        char error[ERROR_BUFFER_LENGTH];
        GLsizei length = 0;
        glGetProgramInfoLog(id, ERROR_BUFFER_LENGTH - 1, &length, error);
        
        // Print error
        cerr << "Shader failed to link:" << endl << error;

        return false;
    }
    return true;
}
