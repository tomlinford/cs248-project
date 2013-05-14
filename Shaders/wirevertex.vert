#version 110

attribute vec3 modelspaceVertex;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(modelspaceVertex, 1);
}
