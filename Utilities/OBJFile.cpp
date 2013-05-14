#include "OBJFile.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

using namespace std;
using namespace glm;

const static string VERTEX("v");
const static string TEXTURE("vt");
const static string NORMAL("vn");
const static string FACE("f");
const static string USE_MATERIAL("usemtl");
const static string MATERIAL_LIBRARY("mtllib");

static bool operator<(const VertexIndex& l, const VertexIndex& r) {
    return l.v < r.v || l.t < r.t || l.n < r.n;
}

/** Parses a single face element */
void parseFaceElement(const string& str, size_t& v, size_t& t, size_t& n)
{
    int components = sscanf(str.c_str(), "%lu/%lu/%lu", &v, &t, &n);
    
    // Vertex, texture, and normal indices all present
    if (components == 3) {
        v--;
        t--;
        n--;
    }
    // Vertex and texture or vertex and normal
    else if (components == 2) {
        v--;
        if (!t) {
            n--;
            t = 0;
        }
        else if (!n) {
            t--;
            n = 0;
        }
    }
    // Only vertex
    else if (components == 1) {
        v--;
        t = n = 0;
    }
}

string getFilePath(const char *filename) {
    string file(filename);
    size_t index = file.rfind('/');
    if (index == string::npos) return string("./");
    return file.substr(0, index + 1);
}

void OBJFile::readFile(vector<vec3>& vertCoords, vector<vec2>& texCoords,
                       vector<vec3>& normals, vector<VertexIndex>&
                       vertexIndices, float& max, const char *filename) {
    ifstream infile(filename);
    string line;

    while (getline(infile, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '\r') continue;
        istringstream ss(line);
        string header;
        ss >> header;

        if (header == VERTEX) {
            float x, y, z;
            ss >> x >> y >> z;
            if (abs(x) > max) max = abs(x);
            if (abs(y) > max) max = abs(y);
            if (abs(z) > max) max = abs(z);
            vertCoords.push_back(vec3(x, y, z));
        } else if (header == TEXTURE) {
            float u, v;
            ss >> u >> v;
            texCoords.push_back(vec2(u, v));
        } else if (header == NORMAL) {
            float dx, dy, dz;
            ss >> dx >> dy >> dz;
            normals.push_back(vec3(dx, dy, dz));
        } else if (header == FACE) {
            string str;
            for (int i = 0; i < 3; i++) {
                ss >> str;
                VertexIndex vi;
                parseFaceElement(str, vi.v, vi.t, vi.n);
                vertexIndices.push_back(vi);
            }
        }
    }
}

/** Map from the vertex indices to normal indices */
map<size_t, vector<size_t> > getNormalIndices(const vector<VertexIndex>& vertexIndices) {
    map<size_t, vector<size_t> > m;
    for (const VertexIndex& vi : vertexIndices) {
        m[vi.v].push_back(vi.n);
    }
    return m;
}

/** Parses a .obj file */
OBJFile::OBJFile(const char *filename)
{
    vector<vec3> vertCoords;
    vector<vec2> texCoords;
    vector<vec3> normals;
    vector<VertexIndex> vertexIndices;
    float max = 0;

    ifstream infile(filename);
    string line;

    readFile(vertCoords, texCoords, normals, vertexIndices, max, filename);

    map<VertexIndex, size_t> indexMap;

    for (int i = 0; i < vertexIndices.size(); i++) {
        VertexIndex vi = vertexIndices[i];
        if (indexMap.count(vi) > 0) {
            indices.push_back(indexMap[vi]);
        } else {
            indexMap[vi] = vertices.size();
            indices.push_back(vertices.size());
			vertices.push_back(vertCoords[vi.v] / max);
            if (!texCoords.empty())
				textures.push_back(texCoords[vi.t]);
            if (!normals.empty())
				this->normals.push_back(normals[vi.n]);
        }
    }
}

/** Generates a Model from a parsed obj file */
Model *OBJFile::GenModel()
{
    ArrayBuffer<vec3> abv(vertices);
    ArrayBuffer<vec2> abt(textures);
    ArrayBuffer<vec3> abn(normals);
	ElementArrayBuffer eab(indices);
	ModelBuffer mb(abv, abt, abn, eab);
    return new Model(mb, Material());
}
