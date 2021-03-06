/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

uniform sampler2D texture;

/* Interpolated vertex position from vertex shader */
varying vec2 texturePosition;

void main() 
{
    // Output vertex color from texture
    gl_FragColor = texture2D(texture, texturePosition);
}