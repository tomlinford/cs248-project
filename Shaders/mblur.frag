/* Motion blur fragment shader. Blurs pixels in the input uniform 
 sampler2D texture and writes the result to an output texture.
 The blur amount is determined by pixel's previous screen space 
 position, which combined with its current position provides
 a velocity vector specifying the direction of the blur.

 See: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch27.html
 for explanation of how this works. */

/* Specifies GLSL version 1.10 - corresponds to OpenGL 2.0 */
#version 120

#define MAX_SAMPLES 200
#define TOLERANCE 0.1
#define M_PI 3.14159265358979323846264

uniform sampler2D sceneColorTexture;
uniform sampler2D sceneDepthTexture;
uniform sampler2D velocityTexture;

/* Interpolated texture positions from vertex shader */
varying vec2 texturePosition;

float gaussianWeight(vec2 position, vec2 center, float numSamples)
{
    float sigma = 0.84089642; // Standard variation
    float coefficient = 1.0 / (2 * M_PI * pow(sigma, 2));
    float x_exponent = -pow(position.x - center.x, 2) / (2 * pow(sigma, 2));
    float y_exponent = -pow(position.y - center.y, 2) / (2 * pow(sigma, 2));
    return coefficient * sigma * exp(x_exponent + y_exponent);
}

void main()
{
    // Calculate the motion vector for this pixel
    vec2 velocity = (texture2D(velocityTexture, texturePosition).xy - 0.5) * 2.0;
    
    // Calculate the blur distance and direction
    float texelSize = 1.0 / 512.0;
    float blur_interval = length(velocity) * texelSize;
    
    // Calculate amount of sampling as a function of pixel speed
    // Less speed --> less blur --> less sampling
    float speed = length(velocity / texelSize);
    int samples = int(min(50 * speed, MAX_SAMPLES));
    velocity = normalize(velocity);
    
    // Calculate final color
    vec4 finalColor = vec4(0.0);
    
    // Eliminate background (speed > 512)
    if (speed < 512)
    {
        finalColor = vec4(0.0);
        float currentDepth = texture2D(sceneDepthTexture, texturePosition).x;
        float sampleCount = 0;
        
        // Sample nearby pixels in direction of velocity
        for(int i = 0; i < samples; i++) {
            vec2 offset = i * blur_interval * velocity;
            float depth = texture2D(sceneDepthTexture, texturePosition + offset).x;
            if (abs(depth - currentDepth) < TOLERANCE || abs(depth - 1.0) < TOLERANCE) {
                finalColor += texture2D(sceneColorTexture, texturePosition + offset);
                sampleCount++;
            }
        }
        for(int i = 0; i < samples; i++) {
            vec2 offset = -i * blur_interval * velocity;
            float depth = texture2D(sceneDepthTexture, texturePosition + offset).x;
            if (abs(depth - currentDepth) < TOLERANCE || abs(depth - 1.0) < TOLERANCE) {
                finalColor += texture2D(sceneColorTexture, texturePosition + offset);
                sampleCount++;
            }
        }
        finalColor.xyz /= sampleCount;
    }
    else {
        finalColor = texture2D(sceneColorTexture, texturePosition);
        //finalColor = vec4(1.0);
    }
    
    gl_FragColor = finalColor;
}