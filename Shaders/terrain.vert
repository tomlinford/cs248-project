#version 110

uniform sampler2D heightField;
uniform mat4 MVP;

attribute vec3 vertexCoordinates;
attribute vec2 textureCoordinates;

varying vec3 normal;
varying vec3 pos;

void main() {
    vec3 normal = vec3(0, 0, 1);
    vec4 height = texture2D(heightField, textureCoordinates.xy);
    float displacement = 0.3 * height.x;

    vec3 position = gl_Vertex.xyz;
    position += displacement * normal;
    
    if (abs(textureCoordinates.x - 0.5) < 0.1)
        position -= displacement * 3.0 * normal;

    gl_Position = MVP * vec4(position, 1);
    pos = position;
}