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

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float rand(float min, float max) {
    return min + rand(textureCoordinates) * (max - min);
}

void main() {
    vec3 normal = vec3(0, 1, 0);
    vec4 height = texture2D(heightField, textureCoordinates);
    float displacement = 3 * (height.x - 0.5);

    vec3 position = vertexCoordinates;
    // position += displacement * normal;
    
    //float diff = abs(position.x * position.x + position.z * position.z - 30.0);
    
    // Radius displacement
    // if (diff < 15.0) {
    //     position.y -= 0.9;
    // }
    // else if (diff < 18.0) {
    //     position.y -= 1.0;
    // }
    // else if (diff < 20.0) {
    //     position.y -= 1.2;
    // }
    // else {
    //     position.y += 1.0;
    // }
    
    // float diff2 = textureCoordinates.y - textureCoordinates.x;
    // if (diff2 > 0.0 && diff > 20.0) {
        // position.y += 5.0 * diff2;
    // }
    position.y += displacement;

    if (illum == 0) {
        position.y += 0.0001;
    }

    // Transform vertex coordinates by MVP
    gl_Position = MVP * vec4(position, 1);
    
    // Pass interpolated vertex position and normals to shader
    vertexPosition = (model * vec4(position, 1)).xyz;
}