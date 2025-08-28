#version 430

out vec4 outColor;
in vec2 TexCoords;
uniform sampler2D tex;

void main() {
    vec3 texCol = texture(tex, TexCoords).rgb;
    outColor = vec4(texCol, 1.0f);
}