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

    FragColor = texel;

}