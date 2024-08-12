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
    FragPos = vec3(model * vec4(aPos, 1.0)); // World coordinates of the vertex
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transformed normal
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0); // Screen coordinates

    // Approximate curvature using normal variation
    vec3 dNormal = normalize(aNormal) - Normal;
    Curvature = length(dNormal) * 100000;

    // Extract camera position
    vec3 camPos = -(transpose(mat3(view)) * vec3(view[3])); // Extract camera position

    // Calculate view direction
    vec3 viewDir = normalize(camPos - FragPos);

    // Calculate the dot product of the view direction and the normal

    vDotN = dot(viewDir, normalize(Normal));
    
}
