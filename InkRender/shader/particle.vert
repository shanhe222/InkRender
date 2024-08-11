#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos; // 传递到几何着色器的片段位置
out vec3 Normal;
out vec4 Color; // 传递到几何着色器的颜色

void main() {
    FragPos = aPos;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Color = vec4 (Normal,1.0);
    //Color = aColor;
    gl_Position = vec4(aPos, 1.0); // 只是为了传递数据，实际位置在几何着色器中设置
}
