/* Velocity buffer fragment shader. Outputs a pixel's screen space
 velocity to a texture. The velocity texture is used for motion blur
 in the mblur fragment shader.
 
 See: http://john-chapman-graphics.blogspot.co.uk/2013/01/per-object-motion-blur.html
 for explanation of how this works. */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

varying vec4 vertexPosition;
varying vec4 previousVertexPosition;

void main()
{
    vec2 currentPos = (vertexPosition.xy / vertexPosition.w) * 0.5 + 0.5;
    vec2 previousPos = (previousVertexPosition.xy / previousVertexPosition.w) * 0.5 + 0.5;
    vec2 velocity = (previousPos - currentPos) / 2.0 + 0.5;
    gl_FragColor = vec4(velocity, 0.0, 0.0);
}