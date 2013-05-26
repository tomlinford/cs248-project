#version 110

#define ATTENUATION_DISTANCE 50.0

uniform vec3 baseColor;

/* Illumination model
 * 0 - Flat
 * 1 - Phong
 */
uniform int illum;

/* Camera position */
uniform vec3 cameraPosition;

/* Light position in camera space */
uniform vec3 lightPosition;

/* Interpolated vertex position from vertex shader */
varying vec3 vertexPosition;

void main()
{
    vec3 final_color;
    if (illum > 0) {
        // Calculate colors
        vec3 ambientColor = 0.3 * baseColor;
        vec3 diffuseColor = baseColor;
        
        // Camera position
        vec3 N = normalize(-cross(dFdx(vertexPosition), dFdy(vertexPosition)));
        vec3 L = normalize(lightPosition - vertexPosition);
        vec3 V = normalize(vertexPosition - cameraPosition);
        vec3 H = normalize(L - V);

        // Calculate ambient
        vec3 ambient = ambientColor;

        // Calculate diffuse
        vec3 diffuse = vec3(0);
        diffuse = clamp(dot(L, N), 0.0, 1.0) * diffuseColor;
        
        // Calculate final color
        final_color = ambient + diffuse;
    } else {
        final_color = vec3(0.0, 0.7, 0.9);
    }
    
    // Attenuation factor
    float distance = length(vertexPosition - lightPosition);
    float attenuation = ((ATTENUATION_DISTANCE - distance) / ATTENUATION_DISTANCE);
    gl_FragColor = vec4(final_color, 1.0);
}