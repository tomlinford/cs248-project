#version 110

varying vec3 pos;
varying vec3 normal;

uniform int illum;

void main() {
    vec3 cameraPosition = vec3(0.0, 0.0, 1.0);
    vec3 lightPosition = vec3(0.0, 1.0, 1.0);

    vec3 final_color;
    vec3 base_color = vec3(0.0, 0.4, 0.5);
    
    if (illum) {
        // Calculate colors
        vec3 ambientColor = 0.3 * base_color;
        vec3 diffuseColor = base_color;
        
        // Camera position
        vec3 V = normalize(pos - cameraPosition);
        vec3 L = normalize(lightPosition - pos);
        vec3 H = normalize(L - V);
        vec3 N = normalize(-cross(dFdx(pos), dFdy(pos)));

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
    
    gl_FragColor = vec4(final_color, 1.0);
    // gl_FragColor = vec4(1, 0, 0, 1);
}