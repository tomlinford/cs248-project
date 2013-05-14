#include "Buffer.h"

#include <climits>
#include <iostream>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;


Buffer::Buffer(GLenum target, PFNGLGENBUFFERSPROC genFunc)
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
	} else if (is_same_type<T, size_t>::value) {
		if (data.size() <= UCHAR_MAX) {
			dataType = GL_UNSIGNED_BYTE;
			GLubyte *arr = toArr<T, GLubyte>(data);
			glBufferData(target, data.size(), arr, GL_STATIC_DRAW);
			delete[] arr;
		} else if (data.size() <= USHRT_MAX) {
			dataType = GL_UNSIGNED_SHORT;
			GLushort *arr = toArr<T, GLushort>(data);
			glBufferData(target, data.size() * 2, arr, GL_STATIC_DRAW);
			delete[] arr;
		} else {
			dataType = GL_UNSIGNED_INT;
			GLuint *arr = toArr<T, GLuint>(data);
			glBufferData(target, data.size() * 4, arr, GL_STATIC_DRAW);
			delete[] arr;
		}
	} else {
		cerr << "unknown data type passed to DataBuffer" << endl;
	}
}

template <typename T>
void DataBuffer<T>::Bind() const {
	glBindBuffer(target, id);
}

template <typename T>
ArrayBuffer<T>::ArrayBuffer(const std::vector<T>& data, GLsizei vertexSize)
	: DataBuffer<T>(data, GL_ARRAY_BUFFER), vertexSize(vertexSize)
{
}

template <typename T>
void ArrayBuffer<T>::Use(Program program) const {
	cerr << "void ArrayBuffer<T>::Use not implemented" << endl;
}

ElementArrayBuffer::ElementArrayBuffer(const std::vector<size_t>& data)
	: DataBuffer<size_t>(data, GL_ELEMENT_ARRAY_BUFFER), size(data.size())
{
}

void ElementArrayBuffer::Draw(GLenum mode) const {
	DataBuffer<size_t>::Bind();
	glDrawElements(mode, size, dataType, 0);
}


