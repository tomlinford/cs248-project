/* Fragment shader for the main scene, before post-processing effects */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

#define ATTENUATION_DISTANCE 50

uniform vec3 baseColor;

/* Illumination model
 * 0 - Flat
 * 1 - Phong
 */
uniform bool illum;

/* Attenuate color based on distance? */
uniform bool attenuate;

/* Force field position */
uniform vec3 fieldPosition;
uniform float fieldRadius;
uniform bool hasField;

/* Light position in camera space */
uniform vec3 lightPosition;

/* Interpolated vertex position from vertex shader */
varying vec3 vertexPosition;
varying vec3 normalPosition;

void main()
{
    vec3 final_color;
    if (illum) {
        vec3 color = baseColor;
        if (hasField && distance(vertexPosition, fieldPosition) < fieldRadius) {
            color += vec3(baseColor.z, 0.0, 0.0);
        }
        
        // Calculate colors
        vec3 ambientColor = 0.1f * color;
        vec3 diffuseColor = color;
        
        // Camera position
        vec3 L = normalize(lightPosition - vertexPosition);
        vec3 N = normalize(-cross(dFdx(vertexPosition), dFdy(vertexPosition)));
        
        // Calculate ambient
        vec3 ambient = ambientColor;
        
        // Calculate diffuse
        vec3 diffuse = vec3(0);
        diffuse = clamp(dot(L, N), 0.0, 1.0) * diffuseColor;
        
        // Calculate final color
        final_color = ambient + diffuse;
    } else {
        vec3 color = baseColor;
        if (distance(vertexPosition, fieldPosition) < 75.0) {
            color += vec3(baseColor.z, 0.0, 0.0);
        }
        final_color = color;
    }

    gl_FragColor = vec4(final_color, 1.0);
    
    if (attenuate) {
        // Attenuation factor
        float distance = length(vertexPosition - lightPosition);
        float attenuation = ((ATTENUATION_DISTANCE - distance) / ATTENUATION_DISTANCE);
        gl_FragColor *= attenuation;
    }
}
