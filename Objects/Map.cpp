#include "Map.h"

#include <unordered_map>

using namespace glm;

const static string VERT_FILENAME = "Shaders/terrain.vert";
const static string FRAG_FILENAME = "Shaders/terrain.frag";

unordered_map<size_t, Model *>existingGrids;

Map::Map(float *heightMap, size_t size, int x, int y) :
    p(VERT_FILENAME, FRAG_FILENAME),
	heightField(size, size, GL_LUMINANCE, heightMap)
{
    M = scale(mat4(1), vec3(20.0));
	M = translate(M, vec3(x * 2, 0, y * 2));
    
	if (existingGrids.count(size) > 0) {
        model = existingGrids[size];
    }
    else  {
        string filename;
        if (size == 64) {
            filename = "Models/grid_32.obj";
        }
        else if (size == 32) {
            filename = "Models/grid_32.obj";
        }
        else if (size == 16) {
            filename = "Models/grid_16.obj";
        }
        
        OBJFile *obj = new OBJFile(filename.c_str());
        model = obj->GenModel();
        existingGrids[size] = model;
        delete obj;
    }
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
    // This line crashes my computer, even on a 16 x 16 grid.
    // Makes no sense. See if it works for you? lol
	// model->Draw(p, GL_LINE_LOOP);
    
	p.SetUniform("illum", 1);
	model->Draw(p, GL_TRIANGLES);
    
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
