/* Blur fragment shader. Used with both the vblur and hblur vertex
   shaders. Should blur information in the input uniform sampler2D 
   texture and write to an output texture. 
 
   See: http://xissburg.com/faster-gaussian-blur-in-glsl/
        for explanation of how this works. */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;

/* Interpolated texture positions from vertex shader */
varying vec2 texturePosition;
varying vec2 weightPositions[14];

void main()
{
    gl_FragColor = vec4(0.0);
    gl_FragColor += texture2D(colorTexture, weightPositions[0])  * 0.0044299121055113265;
    gl_FragColor += texture2D(colorTexture, weightPositions[1])  * 0.00895781211794;
    gl_FragColor += texture2D(colorTexture, weightPositions[2])  * 0.0215963866053;
    gl_FragColor += texture2D(colorTexture, weightPositions[3])  * 0.0443683338718;
    gl_FragColor += texture2D(colorTexture, weightPositions[4])  * 0.0776744219933;
    gl_FragColor += texture2D(colorTexture, weightPositions[5])  * 0.115876621105;
    gl_FragColor += texture2D(colorTexture, weightPositions[6])  * 0.147308056121;
    gl_FragColor += texture2D(colorTexture, texturePosition)     * 0.159576912161;
    gl_FragColor += texture2D(colorTexture, weightPositions[7])  * 0.147308056121;
    gl_FragColor += texture2D(colorTexture, weightPositions[8])  * 0.115876621105;
    gl_FragColor += texture2D(colorTexture, weightPositions[9])  * 0.0776744219933;
    gl_FragColor += texture2D(colorTexture, weightPositions[10]) * 0.0443683338718;
    gl_FragColor += texture2D(colorTexture, weightPositions[11]) * 0.0215963866053;
    gl_FragColor += texture2D(colorTexture, weightPositions[12]) * 0.00895781211794;
    gl_FragColor += texture2D(colorTexture, weightPositions[13]) * 0.0044299121055113265;
}