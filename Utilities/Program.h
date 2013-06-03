#pragma once

#include "../gl.h"

#include <glm/glm.hpp>
#include <map>
#include <string>

#include "Texture.h"
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

    /** Use, unuse the program. */
    void Use() const { glUseProgram(id); }
    void Unuse() const { glBindTexture(GL_TEXTURE_2D, 0); glUseProgram(0); }

    /** Get the GLint associated with the program. */
    GLint GetID() const { return id; }
    
    /** Setters for transformation matrices */
    void SetModel(const glm::mat4& model) const;
    void SetView(const glm::mat4& view) const;
    void SetProjection(const glm::mat4& projection) const;
    void SetMVP(const glm::mat4& mvp) const;
    
    /** Generic setters for uniforms */
    void SetUniform(const char *name, const GLint value) const;
    void SetUniform(const char *name, const GLfloat value) const;
    void SetUniform(const char *name, const glm::vec2& value) const;
    void SetUniform(const char *name, const glm::vec3& value) const;
    void SetUniform(const char *name, const glm::vec4& value) const;
    void SetUniform(const char *name, const glm::mat4& value) const;
    void SetUniform(const char *name, const Texture *texture, GLenum unit) const;
    
    /** Getters for attribute/uniform locations, for
     users who may want more direct control */
    GLint GetAttribLocation(const char *name) const;
    GLint GetUniformLocation(const char *name) const;

	/* Prints all active uniforms for debugging */
	void PrintActiveUniforms() const;

private:
    Program() : id(-1) {}
    
    /** Map of attribute locations */
    mutable map<string, GLint> *locations;

    /** id used by OpenGL. */
    GLint id;
};
