#version 330 core
layout (location = 0) in vec3 aPos; // 顶点位置
layout (location = 1) in vec2 aTexCoords; // 顶点的纹理坐标

out vec2 TexCoords; // 输出到片段着色器的纹理坐标

void main() {
    gl_Position = vec4(aPos, 1.0); // 将顶点位置直接作为裁剪空间坐标
    TexCoords = aTexCoords; // 传递纹理坐标到片段着色器
}
