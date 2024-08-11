#version 330 core
out vec4 FragColor;

in vec2 TexCoords; // 从顶点着色器接收的纹理坐标
uniform sampler2D textureSampler; // 输入的纹理

float noise(vec2 st) {
    // 简单的噪声函数
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 blurEffect(vec2 coords, float radius) {
    vec3 colorSum = vec3(0.0);
    float total = 0.0;
    for(float dx = -radius; dx <= radius; dx++) {
        for(float dy = -radius; dy <= radius; dy++) {
            vec2 offset = vec2(dx, dy);
            float weight = 1.0 - length(offset) / radius;
            colorSum += texture(textureSampler, coords + offset * 0.005).rgb * weight;
            total += weight;
        }
    }
    return colorSum / total;
}

void main() {
    vec3 baseColor = texture(textureSampler, TexCoords).rgb;
    float n = noise(TexCoords * 10.0); // 控制噪声的尺度
    vec3 blurredColor = blurEffect(TexCoords, n * 10.0); // 使用噪声控制模糊半径
    FragColor = vec4(mix(baseColor, blurredColor, 0.5), 1.0);
    //FragColor = vec4(baseColor, 1.0);
}
