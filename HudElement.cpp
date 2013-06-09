#include "HudElement.h"

using namespace::std;
using namespace::glm;

Minimap::Minimap(int x, int y, int w, int h, int win_w, int win_h, Texture *tex, Level *l)
: texture(tex) {
    vector<vec3> vertices;
    vertices.push_back(vec3(x - win_w / 2, y - win_h / 2, 0));
    vertices.push_back(vec3(x - win_w / 2, y + h - win_h / 2, 0));
    vertices.push_back(vec3(x + w - win_w / 2, y + h - win_h / 2, 0));
    vertices.push_back(vec3(x + w - win_w / 2, y - win_h / 2, 0));
    
    vector<vec2> textures;
    textures.push_back(vec2(0, 0));
    textures.push_back(vec2(0, 1));
    textures.push_back(vec2(1, 1));
    textures.push_back(vec2(1, 0));
    
    vector<size_t> indices;
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(3);
    
    ArrayBuffer<vec3> abv(vertices);
    ArrayBuffer<vec2> abt(textures);
    ElementArrayBuffer eab(indices);
    mb = new ModelBuffer(abv, abt, eab);
    
    projection = ortho((float)-win_w / 2, (float)win_w / 2, (float)win_h / 2, (float)-win_h / 2, -0.1f, 1.f);
    
    vector<vec3> outlineVertices;
    outlineVertices.push_back(vec3(x - win_w / 2, y - win_h / 2, 0));
    outlineVertices.push_back(vec3(x - win_w / 2, y + h - win_h / 2, 0));
    outlineVertices.push_back(vec3(x + w - win_w / 2, y + h - win_h / 2, 0));
    outlineVertices.push_back(vec3(x + w - win_w / 2, y - win_h / 2, 0));
    outlineVertices.push_back(vec3(x - win_w / 2, y + h - win_h / 2, 0));
    outlineVertices.push_back(vec3(x + w - win_w / 2, y + h - win_h / 2, 0));
    outlineVertices.push_back(vec3(x - win_w / 2, y - win_h / 2, 0));
    outlineVertices.push_back(vec3(x + w - win_w / 2, y - win_h / 2, 0));
    outlineMB = new ModelBuffer(ArrayBuffer<vec3>(outlineVertices), outlineVertices.size());
    
    vector<vec3> pathVertices;
    for (ControlPoint& cp : l->path) {
        float px = cp.position.x;
        float py = cp.position.z;
        px = px / (16 * 20) * MINIMAP_SIZE;
        py = py / (16 * 20) * MINIMAP_SIZE;
        pathVertices.push_back(vec3(x - win_w / 2 + px, y - win_h / 2 + py, .01));
    }
    pathMB = new ModelBuffer(ArrayBuffer<vec3>(pathVertices), pathVertices.size());
    
    const static float SHIP_SCALE = 15.f;
    vector<vec3> shipVertices;
    shipVertices.push_back(vec3(0, 0, 0) * SHIP_SCALE);
    shipVertices.push_back(vec3(-.3, -.7, 0) * SHIP_SCALE);
    shipVertices.push_back(vec3(1, 0, 0) * SHIP_SCALE);
    shipVertices.push_back(vec3(-.3, .7, 0) * SHIP_SCALE);
    shipMB = new ModelBuffer(ArrayBuffer<vec3>(shipVertices), shipVertices.size());
    shipModel = translate(mat4(1), vec3(x - win_w / 2, y - win_h / 2, 0.02));
}

Minimap::~Minimap() {
    delete texture;
    delete mb;
    delete outlineMB;
    delete pathMB;
    delete shipMB;
}

void Minimap::Draw(const Program &p, Level* l) const {
    p.Use();
    p.SetMVP(projection);
    p.SetUniform("texture", texture, GL_TEXTURE0);
    mb->Draw(p, GL_TRIANGLES);
    
    Program plain("Shaders/plain.vert", "Shaders/plain.frag");
    plain.Use();
    plain.SetUniform("color", vec3(1));
    plain.SetMVP(projection);
    glLineWidth(1.0f);
    outlineMB->Draw(plain, GL_LINES);
    
    pathMB->Draw(plain, GL_LINE_STRIP);
    
    if (l->ship) {
        vec3 shipPos = l->ship->GetPosition();
        vec3 shipDir = l->ship->GetDirection();
        float angle = atan(shipDir.z / shipDir.x) * 180 / glm::pi<float>();
        shipPos /= (16 * 20);
        shipPos *= MINIMAP_SIZE;
        mat4 model = translate(shipModel, vec3(shipPos.x, shipPos.z, 0));
        model = rotate(model, angle, vec3(0, 0, 1));
        plain.SetMVP(projection * model);
        plain.SetUniform("color", vec3(0, 1, 0));
        shipMB->Draw(plain, GL_TRIANGLE_FAN);
    }
    p.Unuse();
}

Reticle::Reticle(int x, int y, int w, int h, int win_w, int win_h, Texture *tex)
: texture(tex)
{
    vector<vec3> vertices;
    vertices.push_back(vec3(x - win_w / 2, y - win_h / 2, 0));
    vertices.push_back(vec3(x - win_w / 2, y + h - win_h / 2, 0));
    vertices.push_back(vec3(x + w - win_w / 2, y + h - win_h / 2, 0));
    vertices.push_back(vec3(x + w - win_w / 2, y - win_h / 2, 0));
    
    vector<vec2> textures;
    textures.push_back(vec2(0, 0));
    textures.push_back(vec2(0, 1));
    textures.push_back(vec2(1, 1));
    textures.push_back(vec2(1, 0));
    
    vector<size_t> indices;
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(3);
    
    ArrayBuffer<vec3> abv(vertices);
    ArrayBuffer<vec2> abt(textures);
    ElementArrayBuffer eab(indices);
    mb = new ModelBuffer(abv, abt, eab);
    
    projection = ortho((float)-win_w / 2, (float)win_w / 2, (float)win_h / 2, (float)-win_h / 2, -0.1f, 1.f);
}

Reticle::~Reticle()
{
    
}

void Reticle::Draw(const Program &p, Level* l) const {
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    p.Use();
    p.SetMVP(projection);
    p.SetUniform("texture", texture, GL_TEXTURE0);
    
    mb->Draw(p, GL_TRIANGLES);
    
    p.Unuse();
    
    glDisable(GL_BLEND);
}
