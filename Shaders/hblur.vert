/* Horizontal blur vertex shader. Expects a screen-aligned quad,
   and corresponding fragment shader output should read from an
   input texture and write to an output texture. 
 
   See: http://xissburg.com/faster-gaussian-blur-in-glsl/
   for explanation of how this works. */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

/* Defined in model space */
attribute vec3 vertexCoordinates;
attribute vec2 textureCoordinates;

/* Interpolated texture coordinates */
varying vec2 texturePosition;
varying vec2 weightPositions[14];

void main()
{
    // Raw position - no transformation
    // This maps the quad directly to the viewport size
    gl_Position = vec4(vertexCoordinates, 1);
    
    // Calculate texture positions to sample here
    // in the vertex shader to avoid dependent texture
    // reads in the fragment shader
    texturePosition = textureCoordinates;
    weightPositions[0]  = textureCoordinates + vec2(-0.014, 0.0);
    weightPositions[1]  = textureCoordinates + vec2(-0.012, 0.0);
    weightPositions[2]  = textureCoordinates + vec2(-0.010, 0.0);
    weightPositions[3]  = textureCoordinates + vec2(-0.008, 0.0);
    weightPositions[4]  = textureCoordinates + vec2(-0.006, 0.0);
    weightPositions[5]  = textureCoordinates + vec2(-0.004, 0.0);
    weightPositions[6]  = textureCoordinates + vec2(-0.002, 0.0);
    weightPositions[7]  = textureCoordinates + vec2( 0.002, 0.0);
    weightPositions[8]  = textureCoordinates + vec2( 0.004, 0.0);
    weightPositions[9]  = textureCoordinates + vec2( 0.006, 0.0);
    weightPositions[10] = textureCoordinates + vec2( 0.008, 0.0);
    weightPositions[11] = textureCoordinates + vec2( 0.010, 0.0);
    weightPositions[12] = textureCoordinates + vec2( 0.012, 0.0);
    weightPositions[13] = textureCoordinates + vec2( 0.014, 0.0);
}