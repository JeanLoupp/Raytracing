#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform image2D imgOutput;

uniform int width;
uniform int height;
uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;


void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    if (pixelCoord.x >= width || pixelCoord.y >= height) {
        return;
    }

    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(width, height);
    uv = uv * 2.0 - 1.0;  // from [0, 1] to [-1, 1]

    vec3 rayDirection = normalize(cameraDirection + uv.x * viewMatrix[0].xyz + uv.y * viewMatrix[1].xyz);

    vec4 finalColor = vec4(1.0);

    imageStore(imgOutput, pixelCoord, finalColor);
}
