#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform sampler2D prevImage;
uniform int frameCount;

uniform int maxBounces;

struct Sphere{
    vec3 pos;
    float r;
    vec3 color;
    vec3 emissionColor;
    float reflexionRatio;
};

struct HitInfo {
    int nextSphere;
    vec3 nextOrigin;
};

uniform int width;
uniform int height;
uniform vec3 cameraPosition;
uniform mat4 viewMatrix;

uniform Sphere spheres[10];
uniform int sphereCount;


// random float between 0 and 1
// https://en.wikipedia.org/wiki/Permuted_congruential_generator
// GLSL uses 32 bit integers
float random(inout uint state){
    uint x = state;
    uint count = uint(x >> 28);

    state = x * 4046619565 + 3654262205;
    x ^= x >> (4 + count);
    x *= 277803737u;
    return float(x ^ (x >> 22)) / 4294967295.0;
}

float randomNormalDist(inout uint state){
    float theta = 2 * 3.14159265359 * random(state);
    float rho = sqrt(-2.0 * log(random(state)));
    return rho * cos(theta);
}

vec3 randomVector(inout uint state){
    vec3 vector;
    vector.x = randomNormalDist(state);
    vector.y = randomNormalDist(state);
    vector.z = randomNormalDist(state);

    return normalize(vector);
}

// https://www.gsn-lib.org/docs/nodes/raytracing.php
vec3 getCameraRay(float fieldOfViewY, float aspectRatio, vec2 point) {
  // compute focal length from given field-of-view
  float focalLength = 1.0 / tan(0.5 * fieldOfViewY * 3.14159265359 / 180.0);
  // compute position in the camera's image plane in range [-1.0, 1.0]
  vec2 pos = 2.0 * (point - 0.5);
  return normalize(vec3(pos.x * aspectRatio, pos.y, -focalLength));
}

HitInfo sendRay(vec3 origin, vec3 direction){
    // direction must be normalized
    HitInfo hitInfo;
    hitInfo.nextSphere = -1;
    float intersection = 1.0 / 0.0;
    for (int i=0; i<sphereCount; i++){
        vec3 pc = spheres[i].pos - origin;
        float proj = dot(pc, direction);
        float det = proj*proj - (dot(pc, pc) - spheres[i].r * spheres[i].r);
        if (det >= 0){
            float t = proj - sqrt(det);
            if (t > 0 && t < intersection){
                intersection = t;
                hitInfo.nextSphere = i;
            }
        }
    }

    hitInfo.nextOrigin = origin + intersection * direction;
    return hitInfo;
}

vec3 getAmbientLight(vec3 direction){
    // direction must be normalized

    vec3 sky = vec3(0.47,0.65,1.0);
    vec3 bottom = vec3(0.2, 0.3, 0.3);

    if (direction.y > 0.1) return sky;
    if (direction.y < -0.1) return bottom;
    
    float t = smoothstep(-0.1, 0.1, direction.y);
    return mix(bottom, sky, t);
}

void main() {

    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    float aspect = float(width) / float(height);

    uint state = pixelCoord.y * width + pixelCoord.x + frameCount * 112067;

    vec3 rayDirection = getCameraRay(45.0, aspect, vec2(pixelCoord) / vec2(width, height));
    rayDirection = (vec4(rayDirection, 1.0) * viewMatrix).xyz;

    vec3 origin = cameraPosition;

    vec3 matColor = vec3(1.0);
    vec3 emiColor = vec3(0.0);

    for (int m=0; m<maxBounces; m++){
        HitInfo hitInfo = sendRay(origin, rayDirection);
        int nextSphere = hitInfo.nextSphere;

        if (nextSphere != -1) {
            origin = hitInfo.nextOrigin;
            vec3 normal = normalize(origin - spheres[nextSphere].pos);

            if (random(state) > spheres[nextSphere].reflexionRatio){
                rayDirection = normalize(normal + randomVector(state));
                if (dot(rayDirection, normal) < 0){
                    rayDirection *= -1.0f;
                }
            } else {
                rayDirection = rayDirection - 2.0*dot(rayDirection, normal) * normal;
            }

            emiColor += spheres[nextSphere].emissionColor * matColor;
            matColor *= spheres[nextSphere].color;

            if (max(matColor.x, max(matColor.y, matColor.z)) < 0.005) break;

        } else {
            emiColor += getAmbientLight(rayDirection) * matColor;
            break;
        }
    }
        
    vec3 prevColor = texelFetch(prevImage, pixelCoord, 0).xyz;

    vec3 finalColor = (prevColor * frameCount + emiColor) / (frameCount + 1);

    imageStore(imgOutput, pixelCoord, vec4(finalColor, 1.0));
}
