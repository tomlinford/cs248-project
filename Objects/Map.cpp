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

Map::Map(float *heightMap, size_t size, int x, int y) :
    p(VERT_FILENAME, FRAG_FILENAME),
	heightField(size, size, GL_LUMINANCE, heightMap)
{
    M = scale(mat4(1), vec3(20.0));
	M = translate(M, vec3(x, 0, y));
    
    // Find position
    position = vec3(20 * x, 0, 20 * y);
    
    // Compute bounds
    ComputeBounds();
    
    // Object destructor will release model
	WirePlane *wp = WirePlane::GetPlane(size);
	triangles = wp->triangleMB;
	lines = wp->lineMB;
}

void Map::ComputeBounds()
{
    int width = heightField.GetWidth();
    int height = heightField.GetHeight();
    GLfloat *data = heightField.GetData();
    
    assert(data != NULL && width > 0 && height > 0);
    
    float minY, maxY;
    minY = maxY = data[0];
    for (int i = 1; i < width * height; i++) {
        if (data[i] < minY) {
            minY = data[i];
        }
        else if (data[i] > maxY) {
            maxY = data[i];
        }
    }
    
    minY = 3.0 * (20 * (minY - 0.5));
    maxY = 3.0 * (20 * (maxY - 0.5));
    
    bounds = Bounds(position + vec3(0, minY, 0),
                    position + vec3(20, maxY, 20));
}

void Map::Draw(const glm::mat4& viewProjection, const glm::vec3& cameraPos, const glm::vec3& lightPos) const {
	p.Use();
    
	p.SetMVP(viewProjection * M);
	p.SetModel(M);
    p.SetUniform("baseColor", color);
    p.SetUniform("lightPosition", lightPos);
    p.SetUniform("cameraPosition", cameraPos);
	p.SetUniform("heightField", &heightField, GL_TEXTURE0);
    
	p.SetUniform("illum", 0);
    lines->Draw(p, GL_LINES);
    
	p.SetUniform("illum", 1);
	triangles->Draw(p, GL_TRIANGLES);
    
#ifdef DEBUG
    p.SetUniform("illum", 0);
    p.SetUniform("baseColor", vec3(1.0));
    DrawAABB(p, viewProjection);
#endif
    
	// this doesn't seem to have any effect and is using just under 4% of the cpu,
	// so I'll just comment it out for now
	//p.Unuse();
}

GLfloat Map::Sample(GLfloat *map, GLuint width, GLuint height, int x, int y)
{
    return map[((y & (height - 1)) * height) + (x & (width - 1))];
}

vec3 Map::GetPosition()
{
    return position;
};

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
