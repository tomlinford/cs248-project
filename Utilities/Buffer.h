#pragma once

#include "../gl.h"

#include <vector>

#include "Program.h"


class Buffer
{
public:
	Buffer(GLenum target, PFNGLGENBUFFERSPROC genFunc);
	~Buffer(void);
	virtual void Bind() const = 0;

protected:
	GLuint id;
	GLenum target;
};

template <typename T>
class DataBuffer : public Buffer {
public:
	DataBuffer(const std::vector<T>& data, GLenum target);
	
	void Bind() const;

protected: 
	GLenum dataType;

};

template <typename T>
class ArrayBuffer : public DataBuffer<T> {
public:
	ArrayBuffer(const std::vector<T>& data, GLsizei vertexSize);
	void Use(Program program) const;

protected:
	GLenum vertexSize;
};

class ElementArrayBuffer : public DataBuffer<size_t> {
public:
	ElementArrayBuffer(const std::vector<size_t>& data);
	void Draw(GLenum mode) const;

protected:
	GLsizei size;
};

