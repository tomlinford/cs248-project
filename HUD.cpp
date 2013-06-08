#include "HUD.h"
#include <sstream>
#include <glm/glm.hpp>

using namespace glm;

static Program *p = NULL;

#define MINIMAP_SIZE 256

// well, this is basically just for the minimap
class HUDElement {
public:
	HUDElement(int x, int y, int w, int h, int win_w, int win_h, Texture *tex, Level *l)
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
	~HUDElement() {
		delete texture;
		delete mb;
		delete outlineMB;
		delete pathMB;
		delete shipMB;
	}

	void Draw(const Program &p, Level* l) const {
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
private:
	Texture *texture;
	ModelBuffer *mb;
	ModelBuffer *outlineMB;
	ModelBuffer *pathMB;
	ModelBuffer *shipMB;
	mat4 shipModel;
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
	for (int x = 0; x < MINIMAP_SIZE; x++) {
		for (int y = 0; y < MINIMAP_SIZE; y++) {
			float xf = (float) x /  (float) MINIMAP_SIZE;
			float yf = (float) y / (float) MINIMAP_SIZE;
			data[x + y * MINIMAP_SIZE] = vec3(0, l->GetHeightAt(xf, yf) * 0.7, l->GetHeightAt(xf, yf) * 0.9);
		}
	}
	return (float *) data;
}

HUD::HUD() : minimap(NULL)
{
    scene = NULL;
	font = new FTPixmapFont("01 Digitall.ttf");
	font->FaceSize(36);
	padding = 10;
}

HUD::~HUD()
{
	delete font;
	delete minimap;
}

void HUD::Render()
{
	if (minimap == NULL) {
		Texture *tex = new Texture(MINIMAP_SIZE, MINIMAP_SIZE, GL_RGB,
			createMinimap(level));
		minimap = new HUDElement(width - MINIMAP_SIZE - 5, height - MINIMAP_SIZE - 5, MINIMAP_SIZE,
			MINIMAP_SIZE, width, height, tex, level);
	}

    if (!p) p = new Program("Shaders/hud.vert", "Shaders/hud.frag");
	minimap->Draw(*p, level);

	glColor4f(1, 1, 1, 1);
	string level = "LEVEL: ";
	level.append(toString(0));
	glWindowPos2f(0, 0);
	font->Render(level.c_str(), -1, FTPoint(padding, height - padding - font->Ascender()));
    
    float h = 0;
    if (HUD::level->ship) {
        h = HUD::level->ship->GetHealth();
        if (h < 0)
            h = 0;
    }
    
    glColor4f(1 - h / 10, h / 10, 0, 1);
    string health = "HEALTH: ";
    health.append(toString(h));
    FTBBox box = font->BBox(health.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
    glWindowPos2f(0, 0);
    font->Render(health.c_str(), -1, FTPoint(width - 2 * padding - box.Upper().X(), height - padding - font->Ascender()));

    glColor4f(1, 1, 1, 1);
	string score = "SCORE: ";
    if (scene) score.append(toString(scene->score));
    box = font->BBox(score.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
    glWindowPos2f(0, 0);
    font->Render(score.c_str(), -1, FTPoint(width / 2 - box.Upper().X() / 2, height - padding - font->Ascender()));
}