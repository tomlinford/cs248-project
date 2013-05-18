/* Vertex shader for the main scene, before post-processing effects */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

/* Defined in model space */
attribute vec3 vertexCoordinates;
attribute vec3 normalCoordinates;
attribute vec3 textureCoordinates;

/* MVP information */
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 MVP;

/* Interpolated normal & vertex coordinates */
varying vec3 vertexPosition;
varying vec3 normalPosition;

void main()
{
    // Transform vertex coordinates by MVP
    gl_Position = MVP * vec4(vertexCoordinates, 1);
    
    // Pass interpolated vertex position and normals to shader
    vertexPosition = (model * vec4(vertexCoordinates, 1)).xyz;
    normalPosition = (model * vec4(normalCoordinates, 1)).xyz;
    
    // Vertex's output color - no change
    gl_FrontColor = gl_Color;
}
