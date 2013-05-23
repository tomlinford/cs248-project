#include "Map.h"

#include <unordered_map>

using namespace glm;

class WirePlane {
public:
	static WirePlane *GetPlane(size_t size) {
		if (computedPlanes.count(size) > 0) {
			return computedPlanes[size];
		}
		computedPlanes[size] = new WirePlane(size);
		return computedPlanes[size];
	}
	ModelBuffer *triangleMB;
	ModelBuffer *lineMB;
private:

	WirePlane(size_t size) {
		vector<vec3> vertices;
		vector<vec2> textures;
		vector<size_t> triangleIndices;
		vector<size_t> lineIndices;
		
		float increment = 1.f / (size - 1);
		for (size_t i = 0; i < size; i++) {
			for (size_t j = 0; j < size; j++) {
				vertices.push_back(vec3(i * increment, 0, j * increment));
				textures.push_back(vec2(i * increment + (increment / 2),
										j * increment + (increment / 2)));
			}
		}
		for (size_t i = 0; i < size - 1; i++) {
			for (size_t j = 0; j < size - 1; j++) {
				triangleIndices.push_back(i * size + j);
				triangleIndices.push_back((i + 1) * size + j);
				triangleIndices.push_back(i * size + (j + 1));
				triangleIndices.push_back((i + 1) * size + j);
				triangleIndices.push_back(i * size + (j + 1));
				triangleIndices.push_back((i + 1) * size + (j + 1));

				lineIndices.push_back(i * size + j);
				lineIndices.push_back((i + 1) * size + j);
				lineIndices.push_back((i + 1) * size + j);
				lineIndices.push_back((i + 1) * size + (j + 1));
				lineIndices.push_back((i + 1) * size + (j + 1));
				lineIndices.push_back(i * size + (j + 1));
				lineIndices.push_back(i * size + (j + 1));
				lineIndices.push_back(i * size + j);
			}
		}

		ArrayBuffer<vec3> vertexBuf(vertices);
		ArrayBuffer<vec2> texBuf(textures);
		ElementArrayBuffer triangleElements(triangleIndices);
		ElementArrayBuffer lineElements(lineIndices);

		triangleMB = new ModelBuffer(vertexBuf, texBuf, triangleElements);
		lineMB = new ModelBuffer(vertexBuf, texBuf, lineElements);
	}

	static unordered_map<size_t, WirePlane *> computedPlanes;
};

unordered_map<size_t, WirePlane *> WirePlane::computedPlanes;

const static string VERT_FILENAME = "Shaders/terrain.vert";
const static string FRAG_FILENAME = "Shaders/terrain.frag";

// TODO: Implement
#pragma mark -
Map::Map(float *heightMap, size_t size, int x, int y) : p(VERT_FILENAME, FRAG_FILENAME)
	, heightField(size, size, GL_LUMINANCE, heightMap)
{
	modelMat = translate(mat4(1), vec3(x, 0, y));
	WirePlane *wp = WirePlane::GetPlane(size);
	triangleMB = wp->triangleMB;
	lineMB = wp->lineMB;
}

void Map::Draw(const glm::mat4& viewProjection, const glm::vec3& cameraPos) const {
	p.Use();
    
	p.SetMVP(viewProjection * modelMat);
	p.SetModel(modelMat);
    p.SetUniform("baseColor", color);
	p.SetUniform("heightField", &heightField, GL_TEXTURE0);
    
	p.SetUniform("illum", 0);
	lineMB->Draw(p, GL_LINES);
    
	p.SetUniform("illum", 1);
	triangleMB->Draw(p, GL_TRIANGLES);
    
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
