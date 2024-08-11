// ParticleSystem.cpp
#include "ParticleSystem.h"
#include <stb_image.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp> // 包含 value_ptr 函数
#include <iostream>

ParticleSystem::ParticleSystem(int maxParticles, unsigned int shaderProgram)
    : maxParticles(maxParticles), shaderProgram(shaderProgram) {
    particles.resize(maxParticles);
}

ParticleSystem::~ParticleSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (ParticlebrushTexture) glDeleteTextures(1, &ParticlebrushTexture);
}

// 定义曲线上的点
std::vector<glm::vec3> generateSineWave(int numPoints, float amplitude, float frequency, float phase) {
    std::vector<glm::vec3> points;
    for (int i = 0; i < numPoints; ++i) {
        float x = static_cast<float>(i) / (numPoints - 1);
        float y = amplitude * sin(frequency * x * 2.0f * 3.14159265358979323846f + phase);
        points.push_back(glm::vec3(x, y, 0.0f));
    }
    return points;
}

// 示例生成100个点的正弦波
std::vector<glm::vec3> contour = generateSineWave(10000, 0.1f, 1.0f, 0.0f);

void ParticleSystem::init() {

    int numContourPoints = contour.size();
    for (int i = 0; i < maxParticles; ++i) {
        // 随机选择一个轮廓点
        int contourIndex = rand() % numContourPoints;
        particles[i].position = contour[contourIndex];

        particles[i].velocity = glm::vec3(0.0f, 1.0f, 0.0f);
        particles[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // 初始颜色为红色
        //particles[i].life = static_cast<float>(rand()) / RAND_MAX * 2.0f;
        particles[i].life = 20.0f; // 固定生命周期为2秒
        particles[i].pressure = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // 随机生成0到1之间的压力值
        particles[i].angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::two_pi<float>(); // 随机生成角度
    }
    initBuffers();
}


void ParticleSystem::initBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * maxParticles, &particles[0], GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, velocity));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParticleSystem::setParticlesFromMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals) {
    int particleCount = std::min(static_cast<int>(positions.size()), maxParticles);
    int numContourPoints = contour.size();
    for (int i = 0; i < particleCount; ++i) {
        //particles[i].position = positions[i];
        //particles[i].velocity = normals[i]; // 可以根据法线生成粒子的速度方向
        particles[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // 初始颜色为红色
        //particles[i].life = static_cast<float>(rand()) / RAND_MAX * 2.0f; // 随机生命周期在0到2秒之间
        int contourIndex = rand() % numContourPoints;
        particles[i].position = contour[contourIndex];

        // 生成随机速度
        // 生成随机速度
        float angleXY = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846f; // 随机XY平面方向
        float speedXY = static_cast<float>(rand()) / RAND_MAX * 0.1f; // 随机XY平面速度大小
        float speedZ = static_cast<float>(rand()) / RAND_MAX * 0.1f - 0.05f; // 随机Z方向速度大小，范围在[-0.05, 0.05]
        particles[i].velocity = glm::vec3(cos(angleXY) * speedXY, sin(angleXY) * speedXY, speedZ);

        particles[i].life = static_cast<float>(rand()) / RAND_MAX * 50.0f;
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * maxParticles, &particles[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystem::update(float deltaTime,
    const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model) {
    
    /*for (int i = 0; i < maxParticles; ++i) {
        // 更新位置
        particles[i].position += particles[i].velocity * deltaTime;

        // 更新寿命
        particles[i].life -= deltaTime;

        if (particles[i].life <= 0.0) {
            // 重新生成粒子
            particles[i].position = glm::vec3(
                static_cast<float>(rand()) / RAND_MAX,
                static_cast<float>(rand()) / RAND_MAX,
                static_cast<float>(rand()) / RAND_MAX
            );

            float angleXY = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846f; // 随机XY平面方向
            float speedXY = static_cast<float>(rand()) / RAND_MAX * 0.1f; // 随机XY平面速度大小
            float speedZ = static_cast<float>(rand()) / RAND_MAX * 0.1f - 0.05f; // 随机Z方向速度大小，范围在[-0.05, 0.05]
            particles[i].velocity = glm::vec3(cos(angleXY) * speedXY, sin(angleXY) * speedXY, speedZ);

            particles[i].life = static_cast<float>(rand()) / RAND_MAX * 5.0f;
        }
    }*/

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * maxParticles, &particles[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystem::render(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model) {
    glUseProgram(shaderProgram);

    // 设置投影和视图矩阵的 uniform
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // 激活并绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ParticlebrushTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "brushTexture"), 0);

    //绘制粒子
    glBindVertexArray(VAO);
    //for (int i = 0; i < maxParticles; ++i) {
    //    glUniform1f(glGetUniformLocation(shaderProgram, "pressure"), particles[i].pressure);
    //    glUniform1f(glGetUniformLocation(shaderProgram, "angle"), particles[i].angle);
    //    glDrawArrays(GL_POINTS, i, 1);
    //}
    glDrawArrays(GL_POINTS, 0, maxParticles);
    glBindVertexArray(0);
}
void ParticleSystem::loadTexture(const std::string& filePath) {
    glGenTextures(1, &ParticlebrushTexture);
    glBindTexture(GL_TEXTURE_2D, ParticlebrushTexture);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 加载图像数据到纹理中
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
            format = GL_RGB; // 默认格式

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::cerr << "Successfully loaded particle texture: " << filePath << std::endl;
    }
    else {
        std::cerr << "Failed to load texture at path: " << filePath << std::endl;
    }
    stbi_image_free(data);
}

