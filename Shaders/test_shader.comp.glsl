#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D accumulatioBuffer;
layout(rgba32f, binding = 1) uniform image2D image_out;

uniform int frameCnt;

const float PI = 3.1415926535897932384626433832795;
// Increased EPSILON slightly for more robust surface offset
const float EPSILON = 0.001;
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
    vec4 color;
    vec4 emissionColor;
    float emissionStrength;
    float specular;
};

// --- SPHERE Structure (SSBO binding 1) ---
struct SphereStruct {
    mat4 transform; // Model matrix (Local to World)
    mat4 invTransform; // Inverse Model matrix (World to Local)
    Material mat;
};

struct Triangle {
    vec4 raw_data[6];
};

struct MeshInfo {
    mat4 transform;
    mat4 invTransform;
    vec4 boundsMin;
    vec4 boundsMax;
    uvec4 info;
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

// SSBO 1: Spheres
layout(std430, binding = 1) buffer sphereBuffer {
    int numSpheres;
    SphereStruct spheres[];
};

layout(std430, binding = 2) buffer triangleBuffer {
    int numTriangles;
    Triangle triangles[];
};

layout(std430, binding = 3) buffer meshBuffer {
    int numMeshes;
    MeshInfo meshes[];
};

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
    mat.color = vec4(color, 0);
    mat.emissionStrength = emissionStrength;
    mat.emissionColor = vec4(emissionColor, 0);
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
    return (1.0 - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5f, 0.7f, 1.0f);
}

void intersectSphereLocal(Ray ray, inout HitInfo hit, Material mat) {
    vec3 oc = ray.origin; // Sphere center is (0,0,0) in Local Space
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(ray.direction, oc);
    float c = dot(oc, oc) - 1.0; // Radius is 1.0

    float disc = b * b - 4.0 * a * c;

    if (disc >= 0.0) {
        float t = (-b - sqrt(disc)) / (2.0 * a);

        vec3 pos = ray.origin + ray.direction * t;
        vec3 normal = normalize(pos);

        if(t > 0) {
            hit.distance = t;
            hit.hasHit = true;
            hit.normal = normal;
            hit.mat = mat;
        }
    }
}

void intersectTriangleLocal(Ray ray, inout HitInfo localHit, Triangle triangle, Material mat) {
    vec3 A = triangle.raw_data[0].xyz;
    vec3 B = triangle.raw_data[1].xyz;
    vec3 C = triangle.raw_data[2].xyz;

    // 1. Calculate edges
    vec3 E1 = B - A; // Edge 1
    vec3 E2 = C - A; // Edge 2

    // 2. Begin calculating determinant - D x E2
    vec3 P = cross(ray.direction, E2);
    float det = dot(E1, P);

    // 3. Check for parallel ray (determinant close to zero)
    if (abs(det) < 0.000001) return;

    float invDet = 1.0 / det;

    // 4. Calculate T vector (distance from A to ray origin)
    vec3 T = ray.origin - A;

    // 5. Calculate U parameter and check bounds
    float u = dot(T, P) * invDet;
    if (u < 0.0 || u > 1.0) return;

    // 6. Calculate Q vector
    vec3 Q = cross(T, E1);

    // 7. Calculate V parameter and check bounds
    float v = dot(ray.direction, Q) * invDet;
    if (v < 0.0 || u + v > 1.0) return; // Standard Moller-Trumbore second check

    // 8. Calculate T parameter (distance along the ray)
    float dst = dot(E2, Q) * invDet;

    // 9. Check if triangle is behind the ray origin or too close
    if (dst < EPSILON) return; // Use the global EPSILON

    // Success: Found a valid hit
    localHit.hasHit = true;
    localHit.distance = dst;
    localHit.mat = mat;

    // 10. Interpolate Normal
    float w = 1.0 - u - v; // Barycentric weight
    vec3 norm1 = triangle.raw_data[3].xyz;
    vec3 norm2 = triangle.raw_data[4].xyz;
    vec3 norm3 = triangle.raw_data[5].xyz;
    localHit.normal = normalize(w * norm1 + u * norm2 + v * norm3);
}

HitInfo intersect(Ray ray) {
    HitInfo bestHit = createHitInfo();

    // 1. Sphere Intersections (LS - Ray already transformed)
    for(int i = 0; i < numSpheres; i++) {
        SphereStruct sphere = spheres[i];

        // Transform ray from World Space to Sphere Local Space

        vec3 localOrigin = vec3(sphere.invTransform * vec4(ray.origin, 1));
        vec3 localDirection = vec3(sphere.invTransform * vec4(ray.direction, 0));
        Ray localRay = createRay(localDirection, localOrigin);

        // Perform intersection against the canonical unit sphere
        HitInfo localSphereHit = createHitInfo();
        intersectSphereLocal(localRay, localSphereHit, sphere.mat);

        if (localSphereHit.hasHit && localSphereHit.distance < bestHit.distance) {
            bestHit.distance = localSphereHit.distance;
            bestHit.mat = localSphereHit.mat;
            mat3 normalTransform = transpose(mat3(sphere.invTransform));
            bestHit.normal = normalize(normalTransform * localSphereHit.normal);
            bestHit.hasHit = localSphereHit.hasHit;
        }
    }

    for(int i = 0; i < numMeshes; i++) {
        MeshInfo mesh = meshes[i];

        uint firstTriangle = mesh.info.x;
        uint trianglesInMesh = mesh.info.y;

        vec3 localOrigin = vec3(mesh.invTransform * vec4(ray.origin, 1));
        vec3 localDirection = vec3(mesh.invTransform * vec4(ray.direction, 0));
        Ray localRay = createRay(localDirection, localOrigin);

        for(uint j = firstTriangle; j < firstTriangle + trianglesInMesh; j++) {
            Triangle triangle = triangles[j];

            HitInfo localTriangleHit = createHitInfo();
            intersectTriangleLocal(localRay, localTriangleHit, triangle, mesh.mat);
            if(localTriangleHit.hasHit && localTriangleHit.distance < bestHit.distance) {
                bestHit.distance = localTriangleHit.distance;
                bestHit.mat = localTriangleHit.mat;
                mat3 normalTransform = transpose(mat3(mesh.invTransform));
                bestHit.normal = normalize(normalTransform * localTriangleHit.normal);
                bestHit.hasHit = localTriangleHit.hasHit;
            }
        }
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

        vec3 emittedLight = hit.mat.emissionColor.xyz * hit.mat.emissionStrength;
        finalColor += emittedLight * rayColor;
        rayColor *= hit.mat.color.xyz;

        // Ensure new ray starts slightly outside the surface
        vec3 newPos = (ray.origin + ray.direction * hit.distance) + hit.normal * 0.000001f;

        // Offset origin along the World Space normal
        ray.origin = newPos + hit.normal * EPSILON;

        // Calculate new direction
        vec3 diffuseDir = normalize(hit.normal + RandomUnitVector(seed));
        vec3 specularDir = normalize(reflect(ray.direction, hit.normal));
        vec3 newDir = normalize(mix(diffuseDir, specularDir, hit.mat.specular));

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
        vec3 rayDir = normalize(vec3(View * vec4(viewPointLocal, 0.0f)));

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