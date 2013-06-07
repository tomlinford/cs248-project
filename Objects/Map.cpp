#include "Map.h"
#include <unordered_map>

#define MAP_SCALE_FACTOR 20.0
#define HEIGHT_SCALE_FACTOR 3.0

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
    M = glm::scale(mat4(1), vec3(MAP_SCALE_FACTOR));
	M = translate(M, vec3(x, 0, y));
    
    sphere = NULL;
    
    // Find position
    position = vec3(MAP_SCALE_FACTOR * x, 0, MAP_SCALE_FACTOR * y);
    
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
    
    minY = HEIGHT_SCALE_FACTOR * (MAP_SCALE_FACTOR * (minY - 0.5));
    maxY = HEIGHT_SCALE_FACTOR * (MAP_SCALE_FACTOR * (maxY - 0.5));
    
    bounds = Bounds(position + vec3(0, minY, 0),
                    position + vec3(MAP_SCALE_FACTOR, maxY, MAP_SCALE_FACTOR));
}

void Map::Draw(const glm::mat4& viewProjection, const glm::vec3& cameraPos, const glm::vec3& lightPos, DrawMode mode) const {
	p.Use();
    
	p.SetMVP(viewProjection * M);
	p.SetModel(M);
    p.SetUniform("baseColor", color);
    p.SetUniform("lightPosition", lightPos);
    p.SetUniform("hasField", false);
	p.SetUniform("heightField", &heightField, GL_TEXTURE0);
    
    if (sphere) {
        p.SetUniform("hasField", true);
        p.SetUniform("fieldPosition", sphere->GetPosition());
        p.SetUniform("fieldRadius", sphere->GetScale());
    }
    
	p.SetUniform("illum", 0);
    glLineWidth(2.0f);
    if (mode == GLOW || mode == NORMAL) {
        lines->Draw(p, GL_LINES);
    }
    if (mode == NORMAL) {
        p.SetUniform("illum", 1);
        triangles->Draw(p, GL_TRIANGLES);
    }
    else if (mode == MINIMAP) {
        p.SetUniform("illum", 0);
        triangles->Draw(p, GL_TRIANGLES);
    }
    
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
    return map[((y & (height - 1)) * width) + (x & (width - 1))];
}

bool Map::Intersects(Object& other)
{
    // Check bounding box intersection first
    if (!Object::Intersects(other))
        return false;
    
    // Check height map intersection next
    GLenum format = heightField.GetFormat();
    assert(format == GL_LUMINANCE); // Single component
    
    int width = heightField.GetWidth();
    int height = heightField.GetHeight();
    GLfloat *data = heightField.GetData();
    
    // Get object bounding box in world space
    Bounds bounds = other.GetBounds();
    
    // Check edges of bottom face against height map
    for (int z = bounds.b1.z; z < bounds.f3.z; z++) {
        for (int x = bounds.b1.x; x < bounds.f3.x; x++) {
            int xPos = ((float)x - position.x) * (64.0f / MAP_SCALE_FACTOR);
            int zPos = ((float)z - position.z) * (64.0f / MAP_SCALE_FACTOR);
            
            // Checking this vertex against the wrong map
            // This may occur if the ship bounding box overlaps several maps
            if (xPos < 0 || xPos > 63 || zPos < 0 || zPos > 63)
                continue;
            
            float displacement = Sample(data, width, height, xPos, zPos);
            displacement = HEIGHT_SCALE_FACTOR * (displacement - 0.5); // From vertex shader
            displacement *= MAP_SCALE_FACTOR;
            
            if (displacement > bounds.b1.y)
                return true;
        }
    }
    
    return false;
}
