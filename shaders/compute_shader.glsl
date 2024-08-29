#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 normal;
};

layout(std430, binding = 1) buffer TrianglesBuffer {
    Triangle triangles[];
};

uniform sampler2D prevImage;
uniform int frameCount;

uniform int maxBounces;

struct Material{
    vec3 color;
    vec3 emissionColor;
    float emissionStrength;
    float smoothness;
    float reflexivity;
};

struct Sphere{
    vec3 pos;
    float r;
    Material mat;
};

struct Tore{
    vec3 pos;
    float R;
    float r;
    Material mat;
};

struct TriangleMesh{
    int startIdx;
    int endIdx;
    Material mat;
};

struct HitInfo {
    bool hasHit;
    vec3 nextOrigin;
    vec3 normal;
    Material mat;
};

uniform int width;
uniform int height;
uniform vec3 cameraPosition;
uniform mat4 viewMatrix;

uniform Sphere spheres[10];
uniform int sphereCount;

uniform Tore tores[10];
uniform int toreCount;

uniform TriangleMesh triangleMeshes[10];
uniform int triangleMeshCount;


// random float between 0 and 1
// https://en.wikipedia.org/wiki/Permuted_congruential_generator
// GLSL uses 32 bit integers
float random(inout uint state){
    uint x = state;
    uint count = uint(x >> 28);

    state = x * 4046619565u + 3654262205u;
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

// https://www.shadertoy.com/view/fsB3Wt
float cbrt(in float x) { return sign(x) * pow(abs(x), 1.0 / 3.0); }
int solveQuartic(in float a, in float b, in float c, in float d, in float e, inout vec4 roots) {
    b /= a; c /= a; d /= a; e /= a; // Divide by leading coefficient to make it 1

    // Depress the quartic to x^4 + px^2 + qx + r by substituting x-b/4a
    // This can be found by substituting x+u and the solving for the value
    // of u that makes the t^3 term go away
    float bb = b * b;
    float p = (8.0 * c - 3.0 * bb) / 8.0;
    float q = (8.0 * d - 4.0 * c * b + bb * b) / 8.0;
    float r = (256.0 * e - 64.0 * d * b + 16.0 * c * bb - 3.0 * bb * bb) / 256.0;
    int n = 0; // Root counter

    // Solve for a root to (t^2)^3 + 2p(t^2)^2 + (p^2 - 4r)(t^2) - q^2 which resolves the
    // system of equations relating the product of two quadratics to the depressed quartic
    float ra =  2.0 * p;
    float rb =  p * p - 4.0 * r;
    float rc = -q * q;

    // Depress using the method above
    float ru = ra / 3.0;
    float rp = rb - ra * ru;
    float rq = rc - (rb - 2.0 * ra * ra / 9.0) * ru;

    float lambda;
    float rh = 0.25 * rq * rq + rp * rp * rp / 27.0;
    if (rh > 0.0) { // Use Cardano's formula in the case of one real root
        rh = sqrt(rh);
        float ro = -0.5 * rq;
        lambda = cbrt(ro - rh) + cbrt(ro + rh) - ru;
    }

    else { // Use complex arithmetic in the case of three real roots
        float rm = sqrt(-rp / 3.0);
        lambda = -2.0 * rm * sin(asin(1.5 * rq / (rp * rm)) / 3.0) - ru;
    }

    // Newton iteration to fix numerical problems (using Horners method)
    // Suggested by @NinjaKoala
    for(int i=0; i < 2; i++) {
        float a_2 = ra + lambda;
        float a_1 = rb + lambda * a_2;
        float b_2 = a_2 + lambda;

        float f = rc + lambda * a_1; // Evaluation of λ^3 + ra * λ^2 + rb * λ + rc
        float f1 = a_1 + lambda * b_2; // Derivative

        lambda -= f / f1; // Newton iteration step
    }

    // Solve two quadratics factored from the quartic using the cubic root
    if (lambda < 0.0) return n;
    float t = sqrt(lambda); // Because we solved for t^2 but want t
    float alpha = 2.0 * q / t, beta = lambda + ra;

    float u = 0.25 * b;
    t *= 0.5;

    float z = -alpha - beta;
    if (z > 0.0) {
        z = sqrt(z) * 0.5;
        float h = +t - u;
        roots.xy = vec2(h + z, h - z);
        n += 2;
    }

    float w = +alpha - beta;
    if (w > 0.0) {
        w = sqrt(w) * 0.5;
        float h = -t - u;
        roots.zw = vec2(h + w, h - w);
        if (n == 0) roots.xy = roots.zw;
        n += 2;
    }

    return n;
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
    int nextObj = -1;
    float intersection = 1.0 / 0.0;

    int hitType = -1;

    ////////// SPHERES //////////

    for (int i=0; i<sphereCount; i++){

        vec3 pc = spheres[i].pos - origin;
        float proj = dot(pc, direction);
        float det = proj*proj - (dot(pc, pc) - spheres[i].r * spheres[i].r);
        if (det >= 0){
            float t = proj - sqrt(det);
            if (t > 0 && t < intersection){
                intersection = t;
                nextObj = i;
                hitType = 0;
            }
        }
    }

    ////////// TORES //////////

    for (int i=0; i<toreCount; i++){

        vec3 newOrig = origin - tores[i].pos;

        float cu = dot(newOrig, direction);

        float R = tores[i].R;
        float r = tores[i].r;

        float C = R*R - r*r + dot(newOrig, newOrig);
        float a = 4.0 * cu;
        float b = 4.0 * cu*cu + 2.0 * C +  - 4.0 * R*R * dot(direction.xy, direction.xy);
        float c = 4.0 * C * cu - 8.0 * R*R * dot(newOrig.xy, direction.xy);
        float d = C*C - 4.0 * R*R * dot(newOrig.xy, newOrig.xy);

        vec4 roots;
        int nroots = solveQuartic(1.0, a, b, c, d, roots);

        if (nroots > 0){
            float t = -1;
            for (int j = 0; j < nroots; j++) {
                if (roots[j] > 0) {
                    if (t < 0 || roots[j] < t) {
                        t = roots[j];
                    }
                }
            }

            if (t > 0 && t < intersection){
                intersection = t;
                nextObj = i;
                hitType = 1;
            }
        }
    }

    ////////// TRIANGLES //////////

    vec3 n1, n2, n3, p;
    int triangleHitIdx;

    for (int i=0; i<triangleMeshCount; i++){
        for (int j=triangleMeshes[i].startIdx; j<triangleMeshes[i].endIdx; j++){
            float dirNormal = dot(direction, triangles[j].normal);
            if (dirNormal >= 0) continue;

            float t = dot(triangles[j].v0 - origin, triangles[j].normal) / dirNormal;

            p = origin + t * direction;

            n1 = cross(triangles[j].v1 - triangles[j].v0, p - triangles[j].v0);
            n2 = cross(triangles[j].v2 - triangles[j].v1, p - triangles[j].v1);
            n3 = cross(triangles[j].v0 - triangles[j].v2, p - triangles[j].v2);

            if (dot(n1, n2) >= -0.01 && dot(n2, n3) >= -0.01 && dot(n3, n1) >= -0.01){
                if (t < intersection){
                    intersection = t;
                    hitType = 2;
                    nextObj = i;
                    triangleHitIdx = j;
                }
            }
        }
    }

    if (hitType == -1) {
        hitInfo.hasHit = false;
        return hitInfo;
    }

    hitInfo.hasHit = true;
    hitInfo.nextOrigin = origin + intersection * direction;

    if (hitType == 0){  // Sphere

        hitInfo.mat = spheres[nextObj].mat;
        hitInfo.normal = normalize(hitInfo.nextOrigin - spheres[nextObj].pos);

    } else if (hitType == 1) {  // Tore

        hitInfo.mat = tores[nextObj].mat;

        vec3 translated = hitInfo.nextOrigin - tores[nextObj].pos;
        float commonTerm = dot(translated, translated) - tores[nextObj].r * tores[nextObj].r;
        float R2 = tores[nextObj].R * tores[nextObj].R;
        hitInfo.normal.x = 4.0 * translated.x * (commonTerm - R2);
        hitInfo.normal.y = 4.0 * translated.y * (commonTerm - R2);
        hitInfo.normal.z = 4.0 * translated.z * (commonTerm + R2);

        hitInfo.normal = normalize(hitInfo.normal);

    } else if (hitType == 2) {  // Triangle
        
        hitInfo.mat = triangleMeshes[nextObj].mat;
        hitInfo.normal = triangles[triangleHitIdx].normal;

    }

    hitInfo.nextOrigin += hitInfo.normal * 0.001;

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

    if (pixelCoord.x > width || pixelCoord.y > height) return;

    float aspect = float(width) / float(height);

    uint state = pixelCoord.y * width + pixelCoord.x + frameCount * 112067;

    vec3 rayDirection = getCameraRay(45.0, aspect, vec2(pixelCoord) / vec2(width, height));
    rayDirection = (vec4(rayDirection, 1.0) * viewMatrix).xyz;

    vec3 origin = cameraPosition;

    vec3 matColor = vec3(1.0);
    vec3 emiColor = vec3(0.0);

    for (int m=0; m<maxBounces; m++){
        HitInfo hitInfo = sendRay(origin, rayDirection);

        if (hitInfo.hasHit) {
            origin = hitInfo.nextOrigin;
            vec3 normal = hitInfo.normal;
            Material mat = hitInfo.mat;

            vec3 diffuseDir = normalize(normal + randomVector(state));
            if (dot(diffuseDir, normal) < 0){
                diffuseDir *= -1.0f;
            }
            vec3 specularDir = rayDirection - 2.0*dot(rayDirection, normal) * normal;

            int isReflexive = int(mat.reflexivity > random(state));

            rayDirection = mix(diffuseDir, specularDir, mat.smoothness * isReflexive);
            rayDirection = normalize(rayDirection);

            emiColor += mat.emissionColor * mat.emissionStrength * matColor;
            matColor *= mix(mat.color, vec3(1.0), isReflexive);

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
