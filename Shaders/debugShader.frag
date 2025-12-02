#version 430 core
out vec4 FragColor;

uniform sampler2D u_DebugTexture;
uniform usampler2D u_MeshDebugTexture;
uniform int u_DebugMode;

vec3 hash3(int id) {
    // These constants are often used to spread the integer bits into unique float values
    vec3 p = fract(vec3(id) * vec3(0.1031, 0.11369, 0.13787));
    p += dot(p, p + 33.33);
    return fract(vec3(p.x + p.y, p.y + p.z, p.z + p.x));
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(u_DebugTexture, 0));
    vec4 texel = texture(u_DebugTexture, uv);

    uv.y = 1 - uv.y;

    if (u_DebugMode == 0) {
        // --- 0: Noisy Color ---
        // Display the color directly.
        FragColor = texel;
    }
    else if (u_DebugMode == 1) {
        // --- 1: Depth View (Near is White, Far is Black) ---

        // IMPORTANT: Tune this value based on the furthest distance you want to see clearly.
        // Objects further than this distance will be black (clamped to 0.0).
        const float MAX_VISUAL_DIST = 500.0;

        // 1. Get the raw distance value from the texture's red channel
        float depth = texture(u_DebugTexture, uv).r;

        // 2. Clamp the depth to the maximum viewing distance.
        // This prevents very large (missed ray) or infinite distances from overwhelming the scale.
        float clampedDepth = min(depth, MAX_VISUAL_DIST);

        // 3. Normalize the clamped depth to the [0.0, 1.0] range
        // 0.0 (near) becomes 0.0; MAX_VISUAL_DIST (far) becomes 1.0
        float normalizedDepth = clampedDepth / MAX_VISUAL_DIST;

        // 4. INVERT for visualization: (1.0 - normalizedDepth)
        // 0.0 (near) becomes 1.0 (white); 1.0 (far) becomes 0.0 (black)
        float visDepth = 1.0 - normalizedDepth;

        // 5. Output pure grayscale (R=G=B)
        FragColor = vec4(visDepth, visDepth, visDepth, 1.0);
    }
    else if (u_DebugMode == 2) {
        // --- 2: Normal View ---
        // Normals (in texel.rgb) are usually stored normalized in [-1, 1] range.
        // Map them to the visible [0, 1] range:
        vec3 normal = texel.rgb;
        FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    }
    else if(u_DebugMode == 3) {
        // --- 3: Motion Vector View ---

        // 1. Sample the motion vector (Vx, Vy, Z, W)
        // We assume the motion vector (Vx, Vy) is stored in the R and G channels.
        vec4 motionTexel = texture(u_DebugTexture, uv);
        vec2 motionVector = motionTexel.rg;

        // 2. Map the motion vector from its normalized range (approx [-1, 1])
        // to the visible color range [0, 1].
        // Color = V * 0.5 + 0.5
        vec2 colorVector = motionVector * 0.5 + 0.5;

        // 3. Output the colorized vector. Use the X component for Red and Y for Green.
        // The Blue channel can be set to 0.5 (neutral) or 1.0.
        // Set Blue to 0.5 to keep the center color a neutral gray/green.
        FragColor = vec4(colorVector.x, colorVector.y, 0.5, 1.0);
    }
    else if(u_DebugMode == 4) {
        // Sample the accumulated color (RGB) and the accumulation count (A)
        vec2 flippedUV = vec2(uv.x, 1.0 - uv.y);

        // Sample using the flipped UV
        vec4 accumulatedData = texture(u_DebugTexture, flippedUV);

        // Display the color
        FragColor = vec4(accumulatedData.rgb, 1.0);
    }
    else if(u_DebugMode == 5) {
        vec2 meshUV = vec2(uv.x, uv.y);

        // 2. Sample the R32UI texture (u_MeshIDSampler)
        // texture() returns uvec4 when sampling a usampler2D
        uvec4 meshData = texture(u_MeshDebugTexture, meshUV);
        int meshID = int(meshData.r);

        // 3. Map the ID to a unique color
        vec3 displayColor;

        if (meshID == 0) {
            // ID 0 often means background or no hit. Show as black.
            displayColor = vec3(0.0);
        } else {
            // Use the hash function to get a unique color based on the ID
            displayColor = hash3(meshID);
        }

        FragColor = vec4(displayColor, 1.0);
    }
    else {
        // Default (or error)
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}