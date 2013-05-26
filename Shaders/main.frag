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

/* Camera position */
uniform vec3 cameraPosition;

/* Light position in camera space */
uniform vec3 lightPosition;

/* Interpolated vertex position from vertex shader */
varying vec3 vertexPosition;
varying vec3 normalPosition;

void main()
{
    vec3 final_color;
    if (illum) {
        // Calculate colors
        vec3 ambientColor = 0.1f * baseColor;
        vec3 diffuseColor = baseColor;
        
        // Camera position
        vec3 V = normalize(vertexPosition - cameraPosition);
        vec3 L = normalize(vertexPosition - lightPosition);
        vec3 H = normalize(L + V);
        vec3 N = normalize(cross(dFdx(vertexPosition), dFdy(vertexPosition)));
        
        // Calculate ambient
        vec3 ambient = ambientColor;
        
        // Calculate diffuse
        vec3 diffuse = vec3(0);
        diffuse = clamp(dot(L, N), 0.0, 1.0) * diffuseColor;
        
        // Calculate final color
		// adding H * 0.000001 is a hack to make sure that cameraPosition doesn't get optimized out
        final_color = ambient + diffuse + (H * 0.00001);
    } else {
        final_color = baseColor;
    }
    
    // Attenuation factor
    float distance = length(vertexPosition - lightPosition);
    float attenuation = ((ATTENUATION_DISTANCE - distance) / ATTENUATION_DISTANCE);
    gl_FragColor = attenuation * vec4(final_color, 1.0);
}
