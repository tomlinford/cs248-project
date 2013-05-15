#version 110

/* Defined in model space */
attribute vec3 vertexCoordinates;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vertexCoordinates, 1);
}
