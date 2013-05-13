#include "Buffer.h"

#include <climits>
#include <iostream>

using namespace std;


Buffer::Buffer(GLenum target, void (*genFunc)(GLsizei, GLuint *))
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
	} else if (is_same_type<T, size_t>::value) {
		if (size <= UCHAR_MAX) {
			dataType = GL_UNSIGNED_BYTE;
			GLubyte *arr = toArr<T, GLubyte>(v);
			glBufferData(target, data.size(), arr, GL_STATIC_DRAW);
			delete[] arr;
		} else if (size <= USHRT_MAX) {
			dataType = GL_UNSIGNED_SHORT;
			GLushort *arr = toArr<T, GLushort>(v);
			glBufferData(target, data.size() * 2, arr, GL_STATIC_DRAW);
			delete[] arr;
		} else {
			dataType = GL_UNSIGNED_INT;
			GLuint *arr = toArr<T, GLuint>(v);
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

