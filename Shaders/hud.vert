#version 120

/* Defined in model space */
attribute vec3 vertexCoordinates;
attribute vec2 textureCoordinates;

/* Interpolated texture coordinates */
varying vec2 texturePosition;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(vertexCoordinates, 1);
    texturePosition = textureCoordinates;
}