#include "Map.h"

#include <unordered_map>

using namespace glm;

const static string VERT_FILENAME = "Shaders/terrain.vert";
const static string FRAG_FILENAME = "Shaders/terrain.frag";

static inline void addToVectors(int i, int j, size_t size, float increment, vector<vec3>& vertices,
								vector<vec2>& textures, vector<size_t>& indices,
								unordered_map<size_t, size_t>& indexing) {
	if (indexing.count(i + j * size) == 0) {
		indexing[i + j * size] = vertices.size();
		textures.push_back(vec2(i * increment, j * increment));
		vertices.push_back(vec3((i - (int) size / 2) * increment,
								(j - (int) size / 2) * increment, 0));
	}
	indices.push_back(indexing[i + j * size]);
}

// TODO: Implement
#pragma mark -
Map::Map(float *heightMap, size_t size) : p(VERT_FILENAME, FRAG_FILENAME)
	, heightField(size, size, GL_LUMINANCE, heightMap)
{
	p.PrintActiveUniforms();
	vector<vec3> vertices;
	vector<vec2> textures;
	vector<size_t> triangleIndices;
	vector<size_t> lineIndices;
	unordered_map<size_t, size_t> indexing;

	float halfsize = size / 2;
	float increment = 1.f / size;
	for (int i = 0; i < size - 1; i++) {
		for (int j = 0; j < size - 1; j++) {
			// add everything for the triangles
			addToVectors(i, j, size, increment, vertices, textures, triangleIndices, indexing);
			addToVectors(i + 1, j, size, increment, vertices, textures, triangleIndices, indexing);
			addToVectors(i, j + 1, size, increment, vertices, textures, triangleIndices, indexing);
			addToVectors(i + 1, j, size, increment, vertices, textures, triangleIndices, indexing);
			addToVectors(i, j + 1, size, increment, vertices, textures, triangleIndices, indexing);
			addToVectors(i + 1, j + 1, size, increment, vertices, textures, triangleIndices, indexing);
			
			// add everything for the lines such that we can use GL_LINES
			addToVectors(i, j, size, increment, vertices, textures, lineIndices, indexing);
			addToVectors(i + 1, j, size, increment, vertices, textures, lineIndices, indexing);
			addToVectors(i + 1, j, size, increment, vertices, textures, lineIndices, indexing);
			addToVectors(i + 1, j + 1, size, increment, vertices, textures, lineIndices, indexing);
			addToVectors(i + 1, j + 1, size, increment, vertices, textures, lineIndices, indexing);
			addToVectors(i, j + 1, size, increment, vertices, textures, lineIndices, indexing);
			addToVectors(i, j + 1, size, increment, vertices, textures, lineIndices, indexing);
			addToVectors(i, j, size, increment, vertices, textures, lineIndices, indexing);
			//addToVectors(i + 1, j, size, increment, vertices, textures, lineIndices, indexing);
			//addToVectors(i, j + 1, size, increment, vertices, textures, lineIndices, indexing);
		}
	}

	ArrayBuffer<vec3> vertexBuf(vertices);
	ArrayBuffer<vec2> texBuf(textures);
	ElementArrayBuffer triangleElements(triangleIndices);
	ElementArrayBuffer lineElements(lineIndices);

	glLineWidth(1.);

	model = new Model(ModelBuffer(vertexBuf, texBuf, triangleElements), Material(), Bounds());
	lineMB = new ModelBuffer(vertexBuf, texBuf, lineElements);
}

void Map::Draw(const glm::mat4& viewProjection, const glm::vec3& cameraPos) const {
	p.Use();
	p.SetMVP(viewProjection);
	p.SetUniform("heightField", &heightField, GL_TEXTURE0);
	p.SetUniform("illum", 0);
	lineMB->Draw(p, GL_LINES);
	p.SetUniform("illum", 1);
	Object::Draw(p, viewProjection, cameraPos);
	p.Unuse();
}

GLfloat Map::Sample(GLfloat *map, GLuint width, GLuint height, int x, int y)
{
    return map[((y & (height - 1)) * height) + (x & (width - 1))];
}

bool Map::Intersects(Object other)
{
    GLenum format = heightField.GetFormat();
    assert(format == GL_LUMINANCE); // Single component
    
    int width = heightField.GetWidth();
    int height = heightField.GetHeight();
    GLfloat *data = heightField.GetData();
    
    Bounds bounds = other.GetBounds();
    
    for (int y = bounds.b1.z; y < bounds.f3.z; y++) {
        for (int x = bounds.b1.x; x < bounds.f3.x; x++) {
            if (Sample(data, width, height, x, y) > bounds.b1.z)
                return true;
        }
    }
    
    return false;
}