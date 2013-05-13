#pragma once

#include "../gl.h"

#include <vector>


class Buffer
{
public:
	Buffer(GLenum target, void (*genFunc)(GLsizei, GLuint *));
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

class ArrayBuffer : public DataBuffer<float> {
public:
	ArrayBuffer(std::vector<float>& data, GLsizei vertexSize);
};

