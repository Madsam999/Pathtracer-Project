#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D accumulatioBuffer;
layout(rgba32f, binding = 1) uniform image2D image_out;

uniform int frameCnt;

const float PI = 3.1415926535897932384626433832795;
const float EPSILON = 0.00001;
const int MAX_UINT = 4294967295;

uniform int RayPerPixel;
uniform int MaxRayBounce;
uniform vec3 ViewParams;
uniform mat4 View;
uniform vec3 ViewCenter;
uniform vec3 RedSphereColor;
uniform vec3 LightColor;
uniform bool DarkMode;

struct Material {
    vec3 color;
    vec3 emissionColor;
    float emissionStrength;
    float specular;
};

struct Sphere {
    vec3 center;
    float radius;
    Material mat;
};

struct Ray {
    vec3 direction;
    vec3 origin;
};

struct HitInfo {
    bool hasHit;
    float distance;
    vec4 color;
    vec3 normal;
    Material mat;
};

layout(std430, binding = 1) buffer sphereBuffer {
    int numSpheres;
    Sphere spheres[];
};

// Source of the code:
// https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/

uint wang_hash(inout uint seed)
{
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float RandomFloat01(inout uint state)
{
    return float(wang_hash(state)) / 4294967296.0;
}

vec3 RandomUnitVector(inout uint state)
{
    float z = RandomFloat01(state) * 2.0f - 1.0f;
    float a = RandomFloat01(state) * (2 * PI);
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}

Material createMaterial(vec3 color, vec3 emissionColor, float emissionStrength, float specular) {
    Material mat;
    mat.color = color;
    mat.emissionStrength = emissionStrength;
    mat.emissionColor = emissionColor;
    mat.specular = specular;
    return mat;
}

Ray createRay(vec3 direction, vec3 origin) {
    Ray ray;
    ray.direction = direction;
    ray.origin = origin;
    return ray;
}

HitInfo createHitInfo() {
    HitInfo info;
    info.hasHit = false;
    info.distance = 100000;
    info.color = vec4(0, 0, 0, 1);
    return info;
}

vec3 colorPixel(Ray ray) {
    vec3 unitDir = normalize(ray.direction);
    float a = 0.5 * (unitDir.y + 1);
    return (1 - a) * vec3(1, 1, 1) + a * vec3(0.5f, 0.7f, 1.0f);
}

void intersectSphere(Ray ray, inout HitInfo hit, Sphere sphere) {
    vec3 oc = sphere.center - ray.origin;
    float a = dot(ray.direction, ray.direction);
    float b = -2 * dot(ray.direction, oc);
    float c = dot(oc, oc) - (sphere.radius * sphere.radius);
    float disc = b * b - 4 * a * c;
    if(disc >= 0) {
        float t = (-b - sqrt(disc)) / (2 * a);
        vec3 pos = ray.origin + ray.direction * t;
        vec3 normal = normalize(pos - sphere.center);
        if(t > 0 && t < hit.distance) {
            hit.distance = t;
            hit.hasHit = true;
            hit.mat = sphere.mat;
            hit.normal = normal;
        }
    }
}

HitInfo intersect(Ray ray) {
    HitInfo bestHit = createHitInfo();

    for(int i = 0; i < numSpheres; i++) {
        intersectSphere(ray, bestHit, spheres[i]);
    }

    return bestHit;
}



vec3 trace(Ray ray, inout uint seed) {
    vec3 finalColor = vec3(0, 0, 0);
    vec3 rayColor = vec3(1, 1, 1);

    for(int mrb = 0; mrb < MaxRayBounce; mrb++) {
        HitInfo hit = intersect(ray);

        // Correctly handle the case where the ray misses an object
        if(!hit.hasHit) {
            if(DarkMode) {
                break;
            }
            else {
                finalColor += colorPixel(ray) * rayColor;
                break;
            }
        }

        vec3 emittedLight = hit.mat.emissionColor * hit.mat.emissionStrength;
        finalColor += emittedLight * rayColor;
        rayColor *= hit.mat.color;

        vec3 newPos = ray.origin + ray.direction * hit.distance;
        vec3 diffuseDir = normalize(hit.normal + RandomUnitVector(seed));
        vec3 specularDir = normalize(reflect(ray.direction, hit.normal));
        vec3 newDir = normalize(mix(diffuseDir, specularDir, hit.mat.specular));

        // Update the ray's origin and direction to continue the path
        ray.origin = newPos + hit.normal * EPSILON;
        ray.direction = newDir;
    }

    return finalColor;
}

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 image_size = imageSize(image_out);

    uint rngState = uint(uint(pixelCoords.x) * uint(1973) + uint(pixelCoords.y) * uint(9277)) | uint(1);
    rngState += uint(frameCnt);

    vec3 avgColor = vec3(0, 0, 0);
    for(int rpp = 0; rpp < RayPerPixel; rpp++) {
        uint sample_seed = rngState + rpp; // A simple way to get a different seed for each sample

        float randomX = RandomFloat01(rngState);
        float randomY = RandomFloat01(rngState);
        vec2 uv = vec2((pixelCoords.x + randomX) / image_size.x - 0.5f, (pixelCoords.y + randomY) / image_size.y - 0.5f);

        vec3 viewPointLocal = vec3(uv.x, -uv.y, -1) * ViewParams;
        vec3 rayDir = vec3(View * vec4(viewPointLocal, 0.0f));

        Ray ray = createRay(rayDir, ViewCenter);

        avgColor += trace(ray, rngState);
    }
    avgColor /= RayPerPixel;

    vec4 oldAccumulatedColor = imageLoad(accumulatioBuffer, pixelCoords);
    vec4 accumumulatedColor = oldAccumulatedColor + vec4(avgColor, 0);

    imageStore(accumulatioBuffer, pixelCoords, accumumulatedColor);

    float invFrameCnt = 1 / float(frameCnt);
    vec4 finalNormalizedColor = accumumulatedColor * invFrameCnt;

    vec4 finalColor = clamp(finalNormalizedColor, 0, 1);

    imageStore(image_out, pixelCoords, finalColor);
}