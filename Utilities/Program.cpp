#include "Program.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace glm;

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

/* Setters for transformation matrices */

void Program::SetModel(const glm::mat4& model) const
{
    GLint id = GetUniformLocation("model");
    glUniformMatrix4fv(id, 1, GL_FALSE, &model[0][0]);
}

void Program::SetView(const glm::mat4& view) const
{
    GLint id = GetUniformLocation("view");
    glUniformMatrix4fv(id, 1, GL_FALSE, &view[0][0]);
}

void Program::SetProjection(const glm::mat4& projection) const
{
    GLint id = GetUniformLocation("projection");
    glUniformMatrix4fv(id, 1, GL_FALSE, &projection[0][0]);
}

void Program::SetMVP(const glm::mat4& mvp) const
{
    GLint id = GetUniformLocation("MVP");
    glUniformMatrix4fv(id, 1, GL_FALSE, &mvp[0][0]);
}

/* Generic setters for uniforms */

void Program::SetUniform(const char *name, GLint value) const
{
    GLint id = GetUniformLocation(name);
    if (id < 0)
        return;
    glUniform1i(id, value);
}

void Program::SetUniform(const char *name, GLfloat value) const
{
    GLint id = GetUniformLocation(name);
    if (id < 0)
        return;
    glUniform1f(id, value);
}

void Program::SetUniform(const char *name, const vec2& value) const
{
    GLint id = GetUniformLocation(name);
    if (id < 0)
        return;
    glUniform2f(id, value.x, value.y);
}

void Program::SetUniform(const char *name, const vec3& value) const
{
    GLint id = GetUniformLocation(name);
    if (id < 0)
        return;
    glUniform3f(id, value.x, value.y, value.z);
}

void Program::SetUniform(const char *name, const vec4& value) const
{
    GLint id = GetUniformLocation(name);
    if (id < 0)
        return;
    glUniform4f(id, value.x, value.y, value.z, value.w);
}

void Program::SetUniform(const char *name, const mat4& value) const
{
    GLint id = GetUniformLocation(name);
    if (id < 0)
        return;
    glUniformMatrix4fv(id, 1, false, &value[0][0]);
}

/* We need an extra parameter for textures to specify
   which texture unit to bind our texture to. If we are
   using multiple textures, they must be bound to
   distinct units. */
void Program::SetUniform(const char *name, Texture *texture, GLenum unit) const
{
    GLint id = GetUniformLocation(name);
    if (id < 0)
        return;
    
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, texture->GetID());
    glUniform1i(id, unit - GL_TEXTURE0);
}

/* Getters for attribute/uniform locations, for
 users who may want more direct control */

GLint Program::GetAttribLocation(const char *name) const {
    GLint location = glGetAttribLocation(id, name);
    if (location < 0) {
        cerr << "Attribute " << name << " not found." << endl;
    }
    return location;
}

GLint Program::GetUniformLocation(const char *name) const
{
    GLint location = glGetUniformLocation(id, name);
    if (location < 0) {
        cerr << "Uniform " << name << " not found." << endl;
    }
    return location;
}
