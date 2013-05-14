#pragma once

#include <glm.hpp>
#include <type_ptr.hpp>

#define TOLERANCE 0.000001

// Column major CM spline weight
static const float mat[16] =
{
    0.0, -0.5, 1.0, -0.5, // column 1
    1.0, 0.0, -2.5, 1.5,
    0.0, 0.5, 2.0, -1.5,
    0.0, 0.0, -0.5, 0.5
};

class CMSpline
{
public:
    CMSpline(float p1, float p2, float p3, float p4) {
        GenConstants(p1, p2, p3, p4);
    }
    
    CMSpline(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4) {
        CMSpline::v1 = v1;
        CMSpline::v2 = v2;
        CMSpline::v3 = v3;
        CMSpline::v4 = v4;
        GenConstants(v1, v2, v3, v4);
    }
    
    /** Evaluation function. Expects a parameter 'u' between 0 and 1. */
    
    float evaluate(float u) {
        return c1 + c2 * u + c3 * pow(u, 2) + c4 * pow(u, 3);
    }
    
    glm::vec3 evaluate3D(float u) {
        return glm::vec3(v1.x + v2.x * u + v3.x * pow(u, 2) + v4.x * pow(u, 3),
                         v1.y + v2.y * u + v3.y * pow(u, 2) + v4.y * pow(u, 3),
                         v1.z + v2.z * u + v3.z * pow(u, 2) + v4.z * pow(u, 3));
    }
    
private:
    /** 1D control points */
    float c1, c2, c3, c4;
    
    /** 3D control points */
    glm::vec3 v1, v2, v3, v4;
    
    /** Generate 1D interpolation coefficients */
    void GenConstants(float p1, float p2, float p3, float p4) {
        glm::mat4 matrix = glm::make_mat4(mat);
        glm::vec4 constants = matrix * glm::vec4(p1, p2, p3, p4);
        c1 = (fabs(constants.x) < TOLERANCE) ? 0 : constants.x;
        c2 = (fabs(constants.y) < TOLERANCE) ? 0 : constants.y;
        c3 = (fabs(constants.z) < TOLERANCE) ? 0 : constants.z;
        c4 = (fabs(constants.w) < TOLERANCE) ? 0 : constants.w;
    }
    
    /** Generate 3D interpolation coefficients */
    void GenConstants(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4) {
        glm::mat4 matrix = glm::make_mat4(mat);
        
        // Generate constants for interpolation in x-dir
        glm::vec4 constants = matrix * glm::vec4(p1.x, p2.x, p3.x, p4.x);
        v1.x = (fabs(constants.x) < TOLERANCE) ? 0 : constants.x;
        v2.x = (fabs(constants.y) < TOLERANCE) ? 0 : constants.y;
        v3.x = (fabs(constants.z) < TOLERANCE) ? 0 : constants.z;
        v4.x = (fabs(constants.w) < TOLERANCE) ? 0 : constants.w;
        
        // Generate constants for interpolation in y-dir
        constants = matrix * glm::vec4(p1.y, p2.y, p3.y, p4.y);
        v1.y = (fabs(constants.x) < TOLERANCE) ? 0 : constants.x;
        v2.y = (fabs(constants.y) < TOLERANCE) ? 0 : constants.y;
        v3.y = (fabs(constants.z) < TOLERANCE) ? 0 : constants.z;
        v4.y = (fabs(constants.w) < TOLERANCE) ? 0 : constants.w;
        
        // Generate constants for interpolation in z-dir
        constants = matrix * glm::vec4(p1.z, p2.z, p3.z, p4.z);
        v1.z = (fabs(constants.x) < TOLERANCE) ? 0 : constants.x;
        v2.z = (fabs(constants.y) < TOLERANCE) ? 0 : constants.y;
        v3.z = (fabs(constants.z) < TOLERANCE) ? 0 : constants.z;
        v4.z = (fabs(constants.w) < TOLERANCE) ? 0 : constants.w;
    }
};
