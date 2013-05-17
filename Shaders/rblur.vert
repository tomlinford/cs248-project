/* Radial blur vertex shader. Expects a screen-aligned quad,
   and corresponding fragment shader output should read from an
   input texture and write to an output texture. */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

/* Defined in model space */
attribute vec3 vertexCoordinates;
attribute vec2 textureCoordinates;

/* Blur direction */
uniform bool forward;

/* Interpolated texture coordinates */
varying vec2 texturePosition;
varying vec2 weightPositions[14];

void main()
{
    // Raw position - no transformation
    // This maps the quad directly to the viewport size
    gl_Position = vec4(vertexCoordinates, 1);
    
    // Compute blur direction
    vec2 blur_dir = vec2(0.5, 0.5) - textureCoordinates;
    float blur_interval = (1.0 / 25.0f) * length(blur_dir);
    blur_dir = normalize(blur_dir);
    if (!forward)
        blur_dir *= -1;
    
    // Calculate texture positions to sample here
    // in the vertex shader to avoid dependent texture
    // reads in the fragment shader
    texturePosition     = textureCoordinates - 1 * blur_interval * blur_dir;
    weightPositions[0]  = textureCoordinates - 3 * blur_interval * blur_dir;
    weightPositions[1]  = textureCoordinates - 5 * blur_interval * blur_dir;
    weightPositions[2]  = textureCoordinates - 7 * blur_interval * blur_dir;
    weightPositions[3]  = textureCoordinates - 5 * blur_interval * blur_dir;
    weightPositions[4]  = textureCoordinates - 3 * blur_interval * blur_dir;
    weightPositions[5]  = textureCoordinates - 1 * blur_interval * blur_dir;
    weightPositions[6]  = textureCoordinates;
    weightPositions[7]  = textureCoordinates - 2 * blur_interval * blur_dir;
    weightPositions[8]  = textureCoordinates - 4 * blur_interval * blur_dir;
    weightPositions[9]  = textureCoordinates - 6 * blur_interval * blur_dir;
    weightPositions[10] = textureCoordinates - 8 * blur_interval * blur_dir;
    weightPositions[11] = textureCoordinates - 6 * blur_interval * blur_dir;
    weightPositions[12] = textureCoordinates - 4 * blur_interval * blur_dir;
    weightPositions[13] = textureCoordinates - 2 * blur_interval * blur_dir;
}
