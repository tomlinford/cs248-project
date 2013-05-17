/* Velocity buffer vertex shader. Expects a screen-aligned quad,
 and corresponding fragment shader output should read from an
 input texture and write to an output texture. */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

/* View projection matrices */
uniform mat4 model;
uniform mat4 currentVP;
uniform mat4 previousVP;

/* Defined in model space */
attribute vec3 vertexCoordinates;

/* Interpolated texture coordinates */
varying vec4 vertexPosition;
varying vec4 previousVertexPosition;

void main()
{
    // Transform vertex by both current and previous view projection
    // matrices
    gl_Position = currentVP * model * vec4(vertexCoordinates, 1);
    vertexPosition = currentVP * model * vec4(vertexCoordinates, 1);
    previousVertexPosition = previousVP * model * vec4(vertexCoordinates, 1);
}