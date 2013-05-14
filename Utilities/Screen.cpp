#include "Screen.h"

using namespace::std;
using namespace::glm;

const static GLfloat QUAD_COORDS[] = {
    -1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
};

const static GLuint QUAD_TEX[] = {
    0.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
    1.0, 0.0
};

const static GLubyte QUAD_INDICES[] = {
    0, 2, 1,
    2, 0, 3
};

Screen::Screen()
{
    // Create full-screen quad
    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_COORDS),
                 QUAD_COORDS, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Bind texture buffer
    glGenBuffers(1, &tex);
    glBindBuffer(GL_ARRAY_BUFFER, tex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_TEX), QUAD_TEX, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Bind index buffer
    glGenBuffers(1, &indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QUAD_INDICES),
                 QUAD_INDICES, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Screen::~Screen()
{
    glDeleteBuffers(1, &handle);
    glDeleteBuffers(1, &tex);
    glDeleteBuffers(1, &indices);
}

void Screen::Draw(Program& program, glm::mat4 viewProjection)
{
    vertexID = program.GetAttribLocation("vertexCoordinates");
    if (vertexID < 0)
    {
        cout << "Unable to find vertexCoordinates" << endl;
        return;
    }
    
    glEnableVertexAttribArray(vertexID);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glVertexAttribPointer(vertexID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    textureID = program.GetAttribLocation("textureCoordinates");
    if (textureID < 0)
    {
        cout << "Unable to find textureCoordinates" << endl;
        return;
    }
    
    glEnableVertexAttribArray(textureID);
    glBindBuffer(GL_ARRAY_BUFFER, tex);
    glVertexAttribPointer(textureID, 2, GL_UNSIGNED_INT, GL_FALSE, 0, (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
    
    glDisableVertexAttribArray(vertexID);
    glDisableVertexAttribArray(textureID);
}