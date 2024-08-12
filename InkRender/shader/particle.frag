#version 330 core

in vec2 TexCoord;
in vec4 ParticleColor;
uniform sampler2D brushTexture;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(brushTexture, TexCoord);
    if (texColor.a < 0.1)
        discard; // 丢弃透明度低的片段

    FragColor = ParticleColor * texColor;
}
