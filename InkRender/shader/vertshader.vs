#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out float vDotN; // 输出视线与法线的点积
out float Curvature;//曲率

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0)); // 顶点的世界坐标
    Normal = mat3(transpose(inverse(model))) * aNormal; // 变换后的法线
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0); // 屏幕坐标
   
    // 使用法线变化近似计算曲率
    vec3 dNormal = normalize(aNormal) - Normal;
    Curvature = length(dNormal)*100000;
    

    // 提取相机位置
    vec3 camPos = -(transpose(mat3(view)) * vec3(view[3])); // 提取相机位置

    // 计算视线方向
    vec3 viewDir = normalize(camPos - FragPos);

    // 计算视线与法线的点积
    vDotN = dot(viewDir, normalize(Normal));
    
}
