#version 110

/* MVP information */
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 MVP;

/* Height map information */
uniform sampler2D heightField;
uniform int illum;

attribute vec3 vertexCoordinates;
attribute vec2 textureCoordinates;

/* Interpolated vertex position from vertex shader */
varying vec3 vertexPosition;

void main() {
    vec3 normal = vec3(0, 1, 0);
    vec4 height = texture2D(heightField, textureCoordinates);
    float displacement = 3.0 * (height.x - 0.5);

    vec3 position = vertexCoordinates;
    position.y += displacement;

    //if (illum == 0) {
    //    position.y += 0.0001;
    //}

    // Transform vertex coordinates by MVP
    gl_Position = MVP * vec4(position, 1);
    
    // Pass interpolated vertex position and normals to shader
    vertexPosition = (model * vec4(position, 1)).xyz;
}