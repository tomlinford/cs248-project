#pragma once

#include "../gl.h"

#include <glm/glm.hpp>
#include <string>

#include "Material.h"

/** The shader class serves as a wrapper around a GLint that openGL
    uses to reference a shader. Shaders can be used for multiple
    programs. */
class Shader {
public:
    /** Create a shader given a type and filename. */
    Shader(GLenum type, const std::string& filename);
    // ~Shader() { if (Valid()) glDeleteShader(id); }

    /** Returns whether or not the shader was created successfully.
        Note that on error, information will be outputted to stderr. */
    bool Valid() const { return id >= 0; }

    /** Get the GLint associated with the shader. Should only be used
        by the program class. */
    GLint GetID() const { return id; }

    static Shader WireVertex;
    static Shader WireFragment;

private:
    Shader() : id(-1) {}
    /** id used by OpenGL. */
    GLint id;
};

/** The program class represents a program that can be used to display
    an object. */
class Program {
public:
	Program(const std::string& vertexShaderFilename,
			const std::string& fragmentShaderFilename);
    /** All programs must have vertex and fragment shaders. */
    Program(const Shader& vertexShader, const Shader& fragmentShader);
    // ~Program() { if (Valid()) glDeleteProgram(id); }

    const Program& operator=(const Program& other) const {
        return other;
    }

    /** Checks to see if a program was created successfully.
        @return true if program was successfully created. */
    bool Valid() const { return id >= 0; }

    /** Attach a shader.
        @return true if shader successfully attached and linked. */
    bool AttachShader(const Shader& shader);

    /** Use the program. */
    void Use() const { glUseProgram(id); }

    /** Get Attribute location. */
    GLuint GetAttribLocation(const char *name) const { 
        return glGetAttribLocation(id, name); }

    /** Get Uniform location. */
    GLuint GetUniformLocation(const char *name) const { 
        return glGetUniformLocation(id, name); }

    void UniformMatrix4f(const char *name, const glm::mat4& mat) const
        { glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]); }

    void Uniform2f(const char *name, const glm::vec2& vec) const
        { glUniform2fv(GetUniformLocation(name), 1, &vec[0]); }

    void Uniform3f(const char *name, const glm::vec3& vec) const
        { glUniform3fv(GetUniformLocation(name), 1, &vec[0]); }

    void UniformMaterial(const Material& mat) const {
        glUniform1f(GetUniformLocation("mat.Ns"), mat.Ns);
        glUniform1f(GetUniformLocation("mat.d"), mat.d);
        glUniform3fv(GetUniformLocation("mat.Ka"), 1, &mat.Ka[0]);
        glUniform3fv(GetUniformLocation("mat.Kd"), 1, &mat.Kd[0]);
        glUniform3fv(GetUniformLocation("mat.Ks"), 1, &mat.Ks[0]);
        glUniform3fv(GetUniformLocation("mat.Ke"), 1, &mat.Ke[0]);
    }

    static Program Wire;

private:
    Program() : id(-1) {}

    /** id used by OpenGL. */
    GLint id;
};
