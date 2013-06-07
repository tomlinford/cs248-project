#include "HUD.h"'
#include <sstream>
#include <glm/glm.hpp>

using namespace glm;

#define MINIMAP_SIZE 256

class HUDElement {
public:
	HUDElement(int x, int y, int w, int h, int win_w, int win_h, Texture *tex)
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

		projection = ortho((float)-win_w / 2, (float)win_w / 2, (float)win_h / 2, (float)-win_h / 2, 0.f, 1.f);
		
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
	}
	~HUDElement() { delete texture; delete mb; }

	void Draw(const Program &p) const {
		p.Use();
		p.SetMVP(projection);
		p.SetUniform("texture", texture, GL_TEXTURE0);
		mb->Draw(p, GL_TRIANGLES);

		Program plain("Shaders/plain.vert", "Shaders/plain.frag");
		plain.Use();
		plain.SetMVP(projection);
		outlineMB->Draw(plain, GL_LINES);
	}
private:
	Texture *texture;
	ModelBuffer *mb;
	ModelBuffer *outlineMB;
	mat4 projection;
};

template<typename T>
static string toString(T t) {
	stringstream s;
	s.precision(2);
	s << t;
	return s.str();
}

static float *createMinimap(Level *l) {
	vec3 *data = new vec3[MINIMAP_SIZE * MINIMAP_SIZE];
	for (int i = 0; i < MINIMAP_SIZE * MINIMAP_SIZE; i++) {
		data[i] = vec3(0, 0, 1);
	}
	for (int x = 0; x < MINIMAP_SIZE; x++) {
		for (int y = 0; y < MINIMAP_SIZE; y++) {
			if (x == 201 && y == 200)
				y = 200;
			float xf = (float) x /  (float) MINIMAP_SIZE;
			float yf = (float) y / (float) MINIMAP_SIZE;
			data[x + y * MINIMAP_SIZE] = vec3(0, 0, l->GetHeightAt(xf, yf));
		}
	}
	return (float *) data;
}

HUD::HUD() : minimap(NULL)
{
	font = new FTPixmapFont("01 Digitall.ttf");
	font->FaceSize(36);
	padding = 10;
}

HUD::~HUD()
{
	delete font;
}

void HUD::Render()
{
	if (minimap == NULL) {
		Texture *tex = new Texture(MINIMAP_SIZE, MINIMAP_SIZE, GL_RGB,
									createMinimap(level));
		minimap = new HUDElement(width - MINIMAP_SIZE - 5, height - MINIMAP_SIZE - 5, MINIMAP_SIZE,
								MINIMAP_SIZE, width, height, tex);
	}

	minimap->Draw(Program("Shaders/hud.vert", "Shaders/hud.frag"));

	glColor4f(1, 1, 1, 1);
	string level = "LEVEL: ";
	level.append(toString(0));
	glWindowPos2f(0, 0);
	font->Render(level.c_str(), -1, FTPoint(padding, height - padding - font->Ascender()));
     
    if (HUD::level->ship) {
        float h = HUD::level->ship->GetHealth();
        if (h < 0)
            h = 0;
        glColor4f(1 - h / 10, h / 10, 0, 1);
        string health = "HEALTH: ";
        health.append(toString(h));
        FTBBox box = font->BBox(health.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
        glWindowPos2f(0, 0);
        font->Render(health.c_str(), -1, FTPoint(width - 2 * padding - box.Upper().X(), height - padding - font->Ascender()));
    }

    glColor4f(1, 1, 1, 1);
	string score = "SCORE: ";
    score.append(toString(0));
    FTBBox box = font->BBox(score.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
    glWindowPos2f(0, 0);
    font->Render(score.c_str(), -1, FTPoint(width / 2 - box.Upper().X() / 2, height - padding - font->Ascender()));
}