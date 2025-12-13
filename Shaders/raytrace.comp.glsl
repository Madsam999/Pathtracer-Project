#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D noisyImage;
layout(r32f, binding = 1) uniform image2D depthImage;
layout(rgba32f, binding = 2) uniform image2D normalImage;
layout(r32ui, binding = 3) uniform uimage2D meshIDImage;
layout(rg16f, binding = 4) uniform image2D motionVectorImage;

uniform int frameCnt;

const float PI = 3.1415926535897932384626433832795;
// Increased EPSILON slightly for more robust surface offset
const float EPSILON = 0.001;
const int MAX_UINT = 4294967295;
const uint BACKGROUND_ID = 4000000000u;

uniform int RayPerPixel;
uniform int MaxRayBounce;

uniform mat4 InverseProjection;
uniform mat4 CameraToWorld;
uniform mat4 PrevVP;
uniform mat4 CurrentVP;

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
    mat4 prevTransform;
    mat4 prevInverseTransform;
    Material mat;
    uvec4 objectID;
};

struct Triangle {
    vec4 positionA, positionB, positionC;
    vec4 normalA, normalB, normalC;
};

struct MeshInfo {
    mat4 transform;
    mat4 invTransform;
    mat4 prevTransform;
    mat4 prevInverseTransform;
    vec4 boundsMin;
    vec4 boundsMax;
    uvec4 info;
    Material mat;
    uvec4 objectID;
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
    uint objectID;
    mat4 inverseModelMatrix;
    mat4 previousModel;
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
    info.objectID = -1;
    return info;
}

vec3 colorPixel(Ray ray) {
    vec3 unitDir = normalize(ray.direction);
    float a = 0.5 * (unitDir.y + 1);
    return (1.0 - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5f, 0.7f, 1.0f);
}

void intersectSphereLocal(Ray ray, inout HitInfo hit, Material mat, uint objectID) {
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
            hit.objectID = objectID;
        }
    }
}

bool intersectAABB(Ray ray, vec3 minBound, vec3 maxBound) {
    vec3 invDir = vec3(1.f / ray.direction.x, 1.f / ray.direction.y, 1.f / ray.direction.z);

    vec3 tMin = (minBound - ray.origin) * invDir;
    vec3 tMax = (maxBound - ray.origin) * invDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return tNear <= tFar;
}

void intersectTriangleLocal(Ray ray, inout HitInfo localHit, Triangle triangle, Material mat, uint objectID) {
    vec3 A = triangle.positionA.xyz;
    vec3 B = triangle.positionB.xyz;
    vec3 C = triangle.positionC.xyz;

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
    localHit.objectID = objectID;

    // 10. Interpolate Normal
    float w = 1.0 - u - v; // Barycentric weight
    vec3 norm1 = triangle.normalA.xyz;
    vec3 norm2 = triangle.normalB.xyz;
    vec3 norm3 = triangle.normalC.xyz;
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
        intersectSphereLocal(localRay, localSphereHit, sphere.mat, sphere.objectID.x);

        if (localSphereHit.hasHit && localSphereHit.distance < bestHit.distance) {
            bestHit.distance = localSphereHit.distance;
            bestHit.mat = localSphereHit.mat;
            mat3 normalTransform = transpose(mat3(sphere.invTransform));
            bestHit.normal = normalize(normalTransform * localSphereHit.normal);
            bestHit.hasHit = localSphereHit.hasHit;
            bestHit.inverseModelMatrix = sphere.invTransform;
            bestHit.previousModel = sphere.prevTransform;
        }
    }

    for(int i = 0; i < numMeshes; i++) {
        MeshInfo mesh = meshes[i];

        uint firstTriangle = mesh.info.x;
        uint trianglesInMesh = mesh.info.y;

        vec3 minBound = mesh.boundsMin.xyz;
        vec3 maxBound = mesh.boundsMax.xyz;

        vec3 localOrigin = vec3(mesh.invTransform * vec4(ray.origin, 1));
        vec3 localDirection = vec3(mesh.invTransform * vec4(ray.direction, 0));
        Ray localRay = createRay(localDirection, localOrigin);

        if(!intersectAABB(localRay, minBound, maxBound)) {
            continue;
        }

        for(uint j = firstTriangle; j < firstTriangle + trianglesInMesh; j++) {
            Triangle triangle = triangles[j];

            HitInfo localTriangleHit = createHitInfo();
            intersectTriangleLocal(localRay, localTriangleHit, triangle, mesh.mat, mesh.objectID.x);
            if(localTriangleHit.hasHit && localTriangleHit.distance < bestHit.distance) {
                bestHit.distance = localTriangleHit.distance;
                bestHit.mat = localTriangleHit.mat;
                mat3 normalTransform = transpose(mat3(mesh.invTransform));
                bestHit.normal = normalize(normalTransform * localTriangleHit.normal);
                bestHit.hasHit = localTriangleHit.hasHit;
                bestHit.previousModel = mesh.prevTransform;
                bestHit.inverseModelMatrix = mesh.invTransform;
            }
        }
    }

    return bestHit;
}

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 image_size = imageSize(noisyImage);

    uint rngState = uint(uint(pixelCoords.x) * uint(1973) + uint(pixelCoords.y) * uint(9277)) | uint(1);
    rngState += uint(frameCnt);

    uint sample_seed = rngState; // A simple way to get a different seed for each sample

    float randomX = RandomFloat01(rngState);
    float randomY = RandomFloat01(rngState);

    vec2 screenPos01 = (vec2(pixelCoords) + vec2(randomX, randomY)) / vec2(image_size);

    vec4 clipPos = vec4(screenPos01 * 2 - 1, 1, 1);

    vec4 viewPos = InverseProjection * vec4(clipPos.xy, -1, 1);
    viewPos.xyz /= viewPos.w;

    vec3 rayDir = normalize(vec3(CameraToWorld * vec4(viewPos.xyz, 0.f)));

    Ray ray = createRay(rayDir, ViewCenter);

    HitInfo primaryHit = createHitInfo();
    primaryHit = intersect(ray);

    if(primaryHit.hasHit) {
        imageStore(depthImage, pixelCoords, vec4(primaryHit.distance, 0, 0, 1));
        imageStore(normalImage, pixelCoords, vec4(primaryHit.normal, 1.f));
        imageStore(meshIDImage, pixelCoords, uvec4(primaryHit.objectID, 0, 0, 0));

        vec3 hitPosition = ray.origin + ray.direction * primaryHit.distance;

        vec3 hitInLocal = vec3(primaryHit.inverseModelMatrix * vec4(hitPosition, 1.f));

        vec3 previousPositionOfHit = vec3(primaryHit.previousModel * vec4(hitInLocal, 1.f));

        vec4 prevClip = PrevVP * vec4(previousPositionOfHit, 1.f);

        vec2 prevUV = (prevClip.xy / prevClip.w) * 0.5 + 0.5;

        vec4 currClip = CurrentVP * vec4(hitPosition, 1.f);
        vec2 currUV = (currClip.xy / currClip.w) * 0.5 + 0.5;

        vec2 motionVector;
        motionVector = currUV - prevUV;

        imageStore(motionVectorImage, pixelCoords, vec4(motionVector, 0, 1));
    }
    else {
        imageStore(depthImage, pixelCoords, vec4(100000.f, 0, 0, 1));
        imageStore(normalImage, pixelCoords, vec4(0, 0, 0, 1));
        imageStore(meshIDImage, pixelCoords, uvec4(BACKGROUND_ID, 0, 0, 0));
        imageStore(motionVectorImage, pixelCoords, vec4(0, 0, 0, 1));
    }

    vec3 finalColor = vec3(0, 0, 0);
    vec3 rayColor = vec3(1, 1, 1);

    if(primaryHit.hasHit) {
        vec3 emittedLight = primaryHit.mat.emissionColor.xyz * primaryHit.mat.emissionStrength;
        finalColor += emittedLight * rayColor;
        rayColor *= primaryHit.mat.color.xyz;

        vec3 newPos = (ray.origin + ray.direction * primaryHit.distance) + primaryHit.normal * 0.000001f;
        ray.origin = newPos;
        uint seed = rngState + frameCnt;
        vec3 diffuseDir = normalize(primaryHit.normal + RandomUnitVector(seed));
        vec3 specularDir = normalize(reflect(ray.direction, primaryHit.normal));
        vec3 newDir = normalize(mix(diffuseDir, specularDir, primaryHit.mat.specular));
        ray.direction = newDir;

        for(int i = 1; i < MaxRayBounce; i++) {
            HitInfo hit = intersect(ray);

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

            vec3 newPos = (ray.origin + ray.direction * hit.distance) + hit.normal * 0.000001f;
            ray.origin = newPos;
            uint seed = rngState + frameCnt;
            vec3 diffuseDir = normalize(hit.normal + RandomUnitVector(seed));
            vec3 specularDir = normalize(reflect(ray.direction, hit.normal));
            vec3 newDir = normalize(mix(diffuseDir, specularDir, hit.mat.specular));
            ray.direction = newDir;
        }
    }

    imageStore(noisyImage, pixelCoords, vec4(finalColor, 1));
}