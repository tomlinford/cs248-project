/* Motion blur vertex shader. Expects a screen-aligned quad,
 and corresponding fragment shader output should read from an
 input texture and write to an output texture. */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

/* Defined in model space */
attribute vec3 vertexCoordinates;
attribute vec2 textureCoordinates;

/* Interpolated texture coordinates */
varying vec2 texturePosition;

void main()
{
    // Raw position - no transformation
    // This maps the quad directly to the viewport size
    gl_Position = vec4(vertexCoordinates, 1);
    
    texturePosition = textureCoordinates;
}