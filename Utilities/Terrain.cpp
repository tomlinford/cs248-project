#include "Terrain.h"

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

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

Terrain::Terrain(GLfloat *terrainMap, size_t size) : p(VERT_FILENAME, FRAG_FILENAME){
	heightField = new Texture(size, size, GL_LUMINANCE, terrainMap);

	vector<vec3> vertices;
	vector<vec2> textures;
	vector<size_t> triangleIndices;
	vector<size_t> lineIndices;
	unordered_map<size_t, size_t> indexing;

	float halfsize = size / 2;
	float increment = 1.f / size;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
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
		}
	}

	ArrayBuffer<vec3> vertexBuf(vertices);
	ArrayBuffer<vec2> texBuf(textures);
	ElementArrayBuffer triangleElements(triangleIndices);
	ElementArrayBuffer lineElements(lineIndices);

	triangleMB = new ModelBuffer(vertexBuf, texBuf, triangleElements);
	lineMB = new ModelBuffer(vertexBuf, texBuf, lineElements);
}

Terrain::~Terrain() {
	delete triangleMB;
	delete lineMB;
	delete heightField;
}

void Terrain::Draw(const glm::mat4& viewProjection) {
	p.Use();
	p.SetMVP(viewProjection);
	p.SetUniform("heightField", heightField, GL_TEXTURE0);
	p.SetUniform("illum", 0);
	lineMB->Draw(p, GL_LINES);
	p.SetUniform("illum", 1);
	triangleMB->Draw(p, GL_TRIANGLES);
	p.Unuse();
}