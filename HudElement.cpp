#include "HudElement.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>

using namespace::std;
using namespace::glm;

#ifndef M_PI
#define M_PI 3.14159265359
#endif

void HUDElement::SetProjection(float win_w, float win_h)
{
    projection = ortho(-win_w / 2, win_w / 2, win_h / 2, -win_h / 2, -0.1f, 1.f);
}

CDIndicator::CDIndicator(int w, int h, string text) : text(text)
{
    vector<vec3> vertices;
    vertices.push_back(vec3(0.0f));
    for (int i = 0; i < 719; i++) {
        float angle = -((float) i) / 718.f * 2.f * M_PI;
        vec2 rot = rotate(vec2(w / 2, 0.f), degrees(angle));
        vertices.push_back(vec3(rot, 0.f) + vertices[0]);
    }
    ab = new ArrayBuffer<vec3>(vertices);
    
    plain = new Program("Shaders/plain.vert", "Shaders/plain.frag");
}

CDIndicator::~CDIndicator()
{
    ab->Delete();
    delete ab;
    delete plain;
}

void CDIndicator::Draw(const Program &p, Scene *s) const
{    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    float elapsed = s->GetTime() - s->GetLastLightning();
    
    if (elapsed < 5) {
        plain->Use();
        
        mat4 model = translate(mat4(1), position);
        plain->SetMVP(projection * model);
        plain->SetUniform("color", vec4(1.0));
        
        // Using an ab so we have more control about how much gets drawn
        ab->Use(*plain, "vertexCoordinates");
        ab->Draw(GL_TRIANGLE_FAN, (5.f - elapsed) / 5.f * 720);
        
        glWindowPos2f(0, 0);
        plain->Unuse();
    }
    
    glDisable(GL_BLEND);
}

Minimap::Minimap(int w, int h, Texture *tex, Level *l)
    : texture(tex)
{
    vector<vec3> vertices;
    vertices.push_back(vec3(-w / 2, -h / 2, 0));
    vertices.push_back(vec3(-w / 2, h / 2, 0));
    vertices.push_back(vec3(w / 2, h / 2, 0));
    vertices.push_back(vec3(w / 2, -h / 2, 0));
    
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
    
    vector<vec3> outlineVertices;
    outlineVertices.push_back(vec3(-w / 2, -h / 2, 0));
    outlineVertices.push_back(vec3(w / 2, -h / 2, 0));
    outlineVertices.push_back(vec3(w / 2, h / 2, 0));
    outlineVertices.push_back(vec3(-w / 2, h / 2, 0));
    outlineVertices.push_back(vec3(-w / 2, -h / 2, 0));
    outlineMB = new ModelBuffer(ArrayBuffer<vec3>(outlineVertices), outlineVertices.size());
    
    vector<vec3> pathVertices;
    for (ControlPoint& cp : l->path) {
        float px = cp.position.x;
        float py = cp.position.z;
        px = px / (16 * 20) * MINIMAP_SIZE;
        py = py / (16 * 20) * MINIMAP_SIZE;
        pathVertices.push_back(vec3(-w / 2 + px, -h / 2 + py, .01));
    }
    pathMB = new ModelBuffer(ArrayBuffer<vec3>(pathVertices), pathVertices.size());
    
    const static float SHIP_SCALE = 15.f;
    vector<vec3> shipVertices;
    shipVertices.push_back(vec3(0, 0, 0) * SHIP_SCALE);
    shipVertices.push_back(vec3(-.3, -.7, 0) * SHIP_SCALE);
    shipVertices.push_back(vec3(1, 0, 0) * SHIP_SCALE);
    shipVertices.push_back(vec3(-.3, .7, 0) * SHIP_SCALE);
    shipMB = new ModelBuffer(ArrayBuffer<vec3>(shipVertices), shipVertices.size());
    shipModel = translate(mat4(1), vec3(-w / 2, -h / 2, 0.02));
    
    plain = new Program("Shaders/plain.vert", "Shaders/plain.frag");
}

Minimap::~Minimap()
{
    mb->Delete();
    delete mb;
    delete texture;
    delete outlineMB;
    delete pathMB;
    delete shipMB;
    delete plain;
}

void Minimap::Draw(const Program &p, Scene *s) const
{
    p.Use();
    
    mat4 M = translate(mat4(1), position);
    p.SetMVP(projection * M);
    p.SetUniform("texture", texture, GL_TEXTURE0);
    
    mb->Draw(p, GL_TRIANGLES);
    
    plain->Use();
    plain->SetUniform("color", vec3(1));
    plain->SetMVP(projection * M);
    
    glLineWidth(1.0f);
    outlineMB->Draw(*plain, GL_LINE_STRIP);
    
    pathMB->Draw(*plain, GL_LINE_STRIP);
    
    if (s->level->ship) {
        vec3 shipPos = s->level->ship->GetPosition();
        vec3 shipDir = s->level->ship->GetDirection();
        float angle = atan(shipDir.z / shipDir.x) * 180 / M_PI;
        shipPos /= (16 * 20);
        shipPos *= MINIMAP_SIZE;
        mat4 model = translate(shipModel, vec3(shipPos.x, shipPos.z, 0)) * M;
        model = rotate(model, angle, vec3(0, 0, 1));
        plain->SetMVP(projection * model);
        plain->SetUniform("color", vec3(0, 1, 0));
        
        shipMB->Draw(*plain, GL_TRIANGLE_FAN);
    }
    p.Unuse();
}

Reticle::Reticle(int w, int h, Texture *tex)
: texture(tex)
{
    vector<vec3> vertices;
    vertices.push_back(vec3(-w / 2, -h / 2, 0));
    vertices.push_back(vec3(-w / 2, h / 2, 0));
    vertices.push_back(vec3(w / 2, h / 2, 0));
    vertices.push_back(vec3(w / 2, -h / 2, 0));
    
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
}

Reticle::~Reticle()
{
    mb->Delete();
    delete mb;
    delete texture;
}

void Reticle::Draw(const Program &p, Scene* s) const
{    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    p.Use();
    
    mat4 model = translate(mat4(1), position);
    p.SetMVP(projection * model);
    p.SetUniform("texture", texture, GL_TEXTURE0);
    
    mb->Draw(p, GL_TRIANGLES);
    
    p.Unuse();
    
    glDisable(GL_BLEND);
}
