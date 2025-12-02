#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// The input texture is the accumulated color C_t^h
uniform sampler2D u_FinalTexture;

void main() {
    // Sample the accumulated color (RGB) and the accumulation count (A)
    vec2 flippedUV = vec2(TexCoords.x, 1.0 - TexCoords.y);

    // Sample using the flipped UV
    vec4 accumulatedData = texture(u_FinalTexture, flippedUV);

    // Display the color
    FragColor = vec4(accumulatedData.rgb, 1.0);
}