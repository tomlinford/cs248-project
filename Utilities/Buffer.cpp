#include "Buffer.h"

#include <climits>
#include <iostream>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

#ifndef __APPLE__
Buffer::Buffer(GLenum target, PFNGLGENBUFFERSPROC genFunc)
#else
Buffer::Buffer(GLenum target, void (*genFunc)(GLsizei, GLuint *))
#endif
	: target(target)
{
	genFunc(1, &id);
}


Buffer::~Buffer(void)
{
}

// These are to determine the types at compile time
// see: http://stackoverflow.com/a/4642514
template<class T, class U>
struct is_same_type { static const bool value = false; };
template<class T>
struct is_same_type<T, T> { static const bool value = true; };

// allocate an array to copy values in v to
template <typename T, typename U>
static U *toArr(const vector<T>& v) {
	U *arr = new U[v.size()];
	for (size_t i = 0; i < v.size(); i++) arr[i] = (U) v[i];
	return arr;
}

template <typename T>
DataBuffer<T>::DataBuffer(const std::vector<T>& data, GLenum target)
	: Buffer(target, glGenBuffers)
{
	if (data.empty()) {
		glDeleteBuffers(1, &target);
		cerr << "Warning: empty data passed to DataBuffer constructor" << endl;
		return;
	}
	Bind();
	if(is_same_type<T, float>::value) {
		dataType = GL_FLOAT;
		glBufferData(target, data.size() * 4, &data[0], GL_STATIC_DRAW);
	} else if (is_same_type<T, vec2>::value) {
		dataType = GL_FLOAT;
		glBufferData(target, data.size() * sizeof(vec2), &data[0], GL_STATIC_DRAW);
	} else if (is_same_type<T, vec3>::value) {
		dataType = GL_FLOAT;
		glBufferData(target, data.size() * sizeof(vec3), &data[0], GL_STATIC_DRAW);
	} else {
		cerr << "unknown data type passed to DataBuffer" << endl;
	}
}

DataBuffer<size_t>::DataBuffer(const std::vector<size_t>& data, GLenum target)
	: Buffer(target, glGenBuffers)
{
	if (data.empty()) {
		glDeleteBuffers(1, &target);
		cerr << "Warning: empty data passed to DataBuffer constructor" << endl;
		return;
	}
	Bind();
	if (data.size() <= UCHAR_MAX) {
		dataType = GL_UNSIGNED_BYTE;
		GLubyte *arr = toArr<size_t, GLubyte>(data);
		glBufferData(target, data.size(), arr, GL_STATIC_DRAW);
		delete[] arr;
	} else if (data.size() <= USHRT_MAX) {
		dataType = GL_UNSIGNED_SHORT;
		GLushort *arr = toArr<size_t, GLushort>(data);
		glBufferData(target, data.size() * 2, arr, GL_STATIC_DRAW);
		delete[] arr;
	} else {
		dataType = GL_UNSIGNED_INT;
		GLuint *arr = toArr<size_t, GLuint>(data);
		glBufferData(target, data.size() * 4, arr, GL_STATIC_DRAW);
		delete[] arr;
	}
}

template <typename T>
ArrayBuffer<T>::ArrayBuffer(const std::vector<T>& data)
	: DataBuffer<T>(data, GL_ARRAY_BUFFER)
{
	if(is_same_type<T, float>::value) {
		vertexSize = 1;
	} else if (is_same_type<T, vec2>::value) {
		vertexSize = 2;
	} else if (is_same_type<T, vec3>::value) {
		vertexSize = 3;
	} else {
		cerr << "unknown data type passed to ArrayBuffer" << endl;
	}
}

template <typename T>
void ArrayBuffer<T>::Use(Program program, const std::string& name) const {
	GLint loc = program.GetAttribLocation(name.c_str());
	if (loc < 0) return;
	glEnableVertexAttribArray(loc);
	DataBuffer<T>::Bind();
	glVertexAttribPointer(loc, vertexSize, DataBuffer<T>::dataType, GL_FALSE, 0, 0);
}

template <typename T>
void ArrayBuffer<T>::Unuse(Program program, const std::string& name) const {
	GLint loc = program.GetAttribLocation(name.c_str());
	if (loc < 0) return;
	glDisableVertexAttribArray(loc);
}

ElementArrayBuffer::ElementArrayBuffer(const std::vector<size_t>& data)
	: DataBuffer<size_t>(data, GL_ELEMENT_ARRAY_BUFFER), size(data.size())
{
}

void ElementArrayBuffer::Draw(GLenum mode) const {
	DataBuffer<size_t>::Bind();
	glDrawElements(mode, size, dataType, 0);
}

ModelBuffer::ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
	const ArrayBuffer<glm::vec2>& textureBuffer,
	const ArrayBuffer<glm::vec3>& normalBuffer,
	const ElementArrayBuffer& elementBuffer)
	: vertexBuffer(vertexBuffer), textureBuffer(textureBuffer)
	, normalBuffer(normalBuffer), elementBuffer(elementBuffer)
	, hasTextureBuffer(true), hasNormalBuffer(true)
{
}

ModelBuffer::ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
	const ArrayBuffer<glm::vec3>& normalBuffer,
	const ElementArrayBuffer& elementBuffer)
	: vertexBuffer(vertexBuffer)
	, normalBuffer(normalBuffer), elementBuffer(elementBuffer)
	, hasTextureBuffer(false), hasNormalBuffer(true)
{
}

ModelBuffer::ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
	const ArrayBuffer<glm::vec2>& textureBuffer,
	const ElementArrayBuffer& elementBuffer)
	: vertexBuffer(vertexBuffer), textureBuffer(textureBuffer)
	, elementBuffer(elementBuffer)
	, hasTextureBuffer(true), hasNormalBuffer(false)
{
}

ModelBuffer::ModelBuffer(const ArrayBuffer<glm::vec3>& vertexBuffer,
	const ElementArrayBuffer& elementBuffer)
	: vertexBuffer(vertexBuffer), elementBuffer(elementBuffer)
	, hasTextureBuffer(false), hasNormalBuffer(false)
{
}

void ModelBuffer::Draw(const Program& p, GLenum mode) const {
	vertexBuffer.Use(p, "vertexCoordinates");
	if (hasTextureBuffer)
		textureBuffer.Use(p, "textureCoordinates");
	if (hasNormalBuffer)
		normalBuffer.Use(p, "normalCoordinates");

	elementBuffer.Draw(mode);
}

template class ArrayBuffer<float>;
template class ArrayBuffer<vec2>;
template class ArrayBuffer<vec3>;
