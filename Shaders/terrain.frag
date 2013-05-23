#version 110

varying vec3 vertexPosition;
varying vec3 normal;

uniform vec3 baseColor;
uniform int illum;

void main() {
    vec3 cameraPosition = vec3(0.0, 0.0, 1.0);
    vec3 lightPosition = vec3(0.0, 1.0, 1.0);

    vec4 final_color;
    
    if (illum > 0) {
        // Calculate colors
        vec3 ambientColor = 0.3 * baseColor;
        // ambientColor = 0.3 * vec3(1, 0, 0);
        vec3 diffuseColor = baseColor;
        // diffuseColor = vec3(0, 1, 0);
        
        // Camera position
        vec3 N = normalize(cross(dFdx(vertexPosition), dFdy(vertexPosition)));
        vec3 L = normalize(lightPosition - vertexPosition);
        vec3 V = normalize(vertexPosition - cameraPosition);
        vec3 H = normalize(L - V);

        // Calculate ambient
        vec3 ambient = ambientColor;

        // Calculate diffuse
        vec3 diffuse = vec3(0);
        diffuse = clamp(dot(L, N), 0.0, 1.0) * diffuseColor;
        
        // Calculate final color
        final_color = vec4(ambient + diffuse, 1);
        // final_color = vec4(base_color, 1);
    } else {
        final_color = vec4(0.0, 0.7, 0.9, 0.6);
    }
    
    gl_FragColor = final_color;
    // if (N.x < -0.05 || L.x < 0.0)
    //     gl_FragColor = vec4(1, 0, 0, 1);
}