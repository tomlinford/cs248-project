#pragma once

#include "gl.h"

#include <vector>

#include "Program.h"

class Buffer
{
public:
#ifndef __APPLE__
	Buffer(GLenum target, PFNGLGENBUFFERSPROC genFunc);
#else
    Buffer(GLenum target, void (*genFunc)(GLsizei, GLuint *));
#endif
	Buffer() {}
	~Buffer(void);
	virtual void Bind() const = 0;
    
    // Separate delete method (instead of destructor)
    // to avoid copying issues
    virtual void Delete();

protected:
	GLuint id;
	GLenum target;
    bool valid;
};

template <typename T>
class DataBuffer : public Buffer {
public:
	DataBuffer(const std::vector<T>& data, GLenum target);
	DataBuffer() {}
	
	void Bind() const { glBindBuffer(target, id); }

protected: 
	GLenum dataType;
};

// partial specialization of DataBuffer
template <>
class DataBuffer<size_t> : public Buffer {
public:
	DataBuffer(const std::vector<size_t>& data, GLenum target);
	DataBuffer() {}
	
	void Bind() const { glBindBuffer(target, id); }

protected: 
	GLenum dataType;
};

template <typename T>
class ArrayBuffer : public DataBuffer<T> {
public:
	ArrayBuffer(const std::vector<T>& data);
	ArrayBuffer() {}
	void Use(const Program& program, const char *name) const;
	void Unuse(const Program& program, const char *name) const;
	void Draw(GLenum mode, GLsizei count) const;

protected:
	GLsizei vertexSize;
};

class ElementArrayBuffer : public DataBuffer<size_t> {
public:
	ElementArrayBuffer(const std::vector<size_t>& data);
	ElementArrayBuffer() {}
	void Draw(GLenum mode) const;

protected:
	GLsizei size;
};

class ModelBuffer {
public:
	ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
		const ArrayBuffer<glm::vec2>& textureBuffer,
		const ArrayBuffer<glm::vec3>& normalBuffer,
		const ElementArrayBuffer& elementBuffer);
	ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
		const ArrayBuffer<glm::vec3>& normalBuffer,
		const ElementArrayBuffer& elementBuffer);
	ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
		const ArrayBuffer<glm::vec2>& textureBuffer,
		const ElementArrayBuffer& elementBuffer);
	ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
		const ElementArrayBuffer& elementBuffer);
	ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
		GLsizei count);

	void Draw(const Program& p, GLenum mode) const;
    void Delete();

private:
    bool valid;
	const bool hasTextureBuffer, hasNormalBuffer, hasIndexBuffer;
	GLsizei count;
	ArrayBuffer<glm::vec3> vertexBuffer;
	ArrayBuffer<glm::vec2> textureBuffer;
	ArrayBuffer<glm::vec3> normalBuffer;
	ElementArrayBuffer elementBuffer;
};
