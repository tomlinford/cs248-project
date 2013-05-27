#include "Object.h"

using namespace::glm;

Object::Object()
{
}

Object::Object(Model *m)
{
    assert (m != NULL);
    model = m;
}

Object::Object(const string& filename)
{
    OBJFile obj(filename.c_str());
    model = obj.GenModel();
}

Object::~Object()
{
    if (model)
        delete model;
};

bool Object::Intersects(Object other)
{
    Bounds myBounds = GetBounds();
    Bounds otherBounds = other.GetBounds();
    
    // f3 and b1 correspond to the max and min
    // corners of the bounding boxes, respectively
    if (myBounds.f3.x < otherBounds.b1.x ||
        otherBounds.f3.x < myBounds.b1.x)
        return false;
    if (myBounds.f3.y < otherBounds.b1.y ||
        otherBounds.f3.y < myBounds.b1.y)
        return false;
    if (myBounds.f3.z < otherBounds.b1.z ||
        otherBounds.f3.z < myBounds.b1.z)
        return false;
    
    return true;
}

/* Helper function for finding AABB */
void updateMinMax(vec3& min, vec3& max, vec4 test)
{
    if (test.x < min.x)
        min.x = test.x;
    else if (test.x > max.x)
        max.x = test.x;
    if (test.y < min.y)
        min.y = test.y;
    else if (test.y > max.y)
        max.y = test.y;
    if (test.z < min.z)
        min.z = test.z;
    else if (test.z > max.z)
        max.z = test.z;
}

/** OPTIMIZE: Optimize so this recomputes bounds
  * only when orientation has changed */
#pragma mark -
Bounds Object::GetBounds() const
{
    // Get extremes of transformed bounding vertices
    vec3 min, max;
    
    // Seed min and max to first vertex
    vec4 t = M * vec4(model->bounds.b1, 1.0);
    min.x = max.x = t.x;
    min.y = max.y = t.y;
    min.z = max.z = t.z;
    
    updateMinMax(min, max, M * vec4(model->bounds.b2, 1.0));
    updateMinMax(min, max, M * vec4(model->bounds.b3, 1.0));
    updateMinMax(min, max, M * vec4(model->bounds.b4, 1.0));
    updateMinMax(min, max, M * vec4(model->bounds.f1, 1.0));
    updateMinMax(min, max, M * vec4(model->bounds.f2, 1.0));
    updateMinMax(min, max, M * vec4(model->bounds.f3, 1.0));
    updateMinMax(min, max, M * vec4(model->bounds.f4, 1.0));
    
    return Bounds(min, max);
}

void Object::DrawAABB(const Program& p, const glm::mat4& viewProjection) const
{
    mat4 model = mat4(1);
    mat4 mvp = viewProjection * model;
    
    p.SetModel(model);
    p.SetMVP(mvp);
    
    Bounds bounds = GetBounds();
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(bounds.b1.x, bounds.b1.y, bounds.b1.z);
    glVertex3f(bounds.b2.x, bounds.b2.y, bounds.b2.z);
    glVertex3f(bounds.b3.x, bounds.b3.y, bounds.b3.z);
    glVertex3f(bounds.b4.x, bounds.b4.y, bounds.b4.z);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(bounds.f1.x, bounds.f1.y, bounds.f1.z);
    glVertex3f(bounds.f2.x, bounds.f2.y, bounds.f2.z);
    glVertex3f(bounds.f3.x, bounds.f3.y, bounds.f3.z);
    glVertex3f(bounds.f4.x, bounds.f4.y, bounds.f4.z);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(bounds.b1.x, bounds.b1.y, bounds.b1.z);
    glVertex3f(bounds.f1.x, bounds.f1.y, bounds.f1.z);
    glVertex3f(bounds.f4.x, bounds.f4.y, bounds.f4.z);
    glVertex3f(bounds.b4.x, bounds.b4.y, bounds.b4.z);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(bounds.b2.x, bounds.b2.y, bounds.b2.z);
    glVertex3f(bounds.f2.x, bounds.f2.y, bounds.f2.z);
    glVertex3f(bounds.f3.x, bounds.f3.y, bounds.f3.z);
    glVertex3f(bounds.b3.x, bounds.b3.y, bounds.b3.z);
    glEnd();
}

void Object::Draw(const Program& p, const glm::mat4& viewProjection,
				 const glm::vec3& cameraPos, GLenum mode) const
{
    mat4 MVP = viewProjection * M;
    
    p.SetUniform("baseColor", color);
    p.SetModel(M); // Needed for Phong shading
	p.SetMVP(MVP);
    
	Object::model->Draw(p, mode);
    
//#ifdef DEBUG
    DrawAABB(p, viewProjection);
//#endif
}