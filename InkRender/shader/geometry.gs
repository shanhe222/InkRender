#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

in vec3 FragPos[];
in vec3 Normal[];
in vec4 Color[];

out vec2 TexCoord;
out vec4 ParticleColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//uniform float pressure;
//uniform float angle;

void main() {

    float pressure = 1.0;
    float angle = 1.0;

    for (int i = 0; i < 3; ++i) {
        vec3 viewDir = normalize(vec3(view * vec4(0.0, 0.0, 0.0, 1.0)) - FragPos[i]);
        vec3 normalizedNormal = normalize(Normal[i]);
        float angle = dot(normalizedNormal, viewDir);
        float edgeThreshold = 10.1;

        if (abs(angle) < edgeThreshold) {
            ParticleColor = vec4(1.0, 0.0, 0.0, 1.0); // 示例颜色，可以根据需要修改

            float majorAxis = 0.005 * pressure; // 长轴，压力影响大小
            float minorAxis = 0.005 * pressure; // 短轴，压力影响大小

            vec3 right = normalize(cross(viewDir, normalizedNormal));
            vec3 up = normalize(cross(normalizedNormal, right));

            mat3 rotationMatrix = mat3(right, up, normalizedNormal);

            vec3 offsets[4] = vec3[4](
                rotationMatrix * vec3(-majorAxis, -minorAxis, 0.0),
                rotationMatrix * vec3(majorAxis, -minorAxis, 0.0),
                rotationMatrix * vec3(-majorAxis, minorAxis, 0.0),
                rotationMatrix * vec3(majorAxis, minorAxis, 0.0)
            );

            vec2 texCoords[4] = vec2[4](
                vec2(0.0, 0.0),
                vec2(1.0, 0.0),
                vec2(0.0, 1.0),
                vec2(1.0, 1.0)
            );

            for (int j = 0; j < 4; ++j) {
                gl_Position = projection * view * (vec4(FragPos[i], 1.0) + vec4(offsets[j], 0.0));
                TexCoord = texCoords[j];
                EmitVertex();
            }
            EndPrimitive();
        }
    }
}
