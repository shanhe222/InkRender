// ParticleSystem.cpp
#include "ParticleSystem.h"
#include <stb_image.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp> // ���� value_ptr ����
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

// ���������ϵĵ�
std::vector<glm::vec3> generateSineWave(int numPoints, float amplitude, float frequency, float phase) {
    std::vector<glm::vec3> points;
    for (int i = 0; i < numPoints; ++i) {
        float x = static_cast<float>(i) / (numPoints - 1);
        float y = amplitude * sin(frequency * x * 2.0f * 3.14159265358979323846f + phase);
        points.push_back(glm::vec3(x, y, 0.0f));
    }
    return points;
}

// ʾ������100��������Ҳ�
std::vector<glm::vec3> contour = generateSineWave(10000, 0.1f, 1.0f, 0.0f);

void ParticleSystem::init() {

    int numContourPoints = contour.size();
    for (int i = 0; i < maxParticles; ++i) {
        // ���ѡ��һ��������
        int contourIndex = rand() % numContourPoints;
        particles[i].position = contour[contourIndex];

        particles[i].velocity = glm::vec3(0.0f, 1.0f, 0.0f);
        particles[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // ��ʼ��ɫΪ��ɫ
        //particles[i].life = static_cast<float>(rand()) / RAND_MAX * 2.0f;
        particles[i].life = 20.0f; // �̶���������Ϊ2��
        particles[i].pressure = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // �������0��1֮���ѹ��ֵ
        particles[i].angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::two_pi<float>(); // ������ɽǶ�
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
        //particles[i].velocity = normals[i]; // ���Ը��ݷ����������ӵ��ٶȷ���
        particles[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // ��ʼ��ɫΪ��ɫ
        //particles[i].life = static_cast<float>(rand()) / RAND_MAX * 2.0f; // �������������0��2��֮��
        int contourIndex = rand() % numContourPoints;
        particles[i].position = contour[contourIndex];

        // ��������ٶ�
        // ��������ٶ�
        float angleXY = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846f; // ���XYƽ�淽��
        float speedXY = static_cast<float>(rand()) / RAND_MAX * 0.1f; // ���XYƽ���ٶȴ�С
        float speedZ = static_cast<float>(rand()) / RAND_MAX * 0.1f - 0.05f; // ���Z�����ٶȴ�С����Χ��[-0.05, 0.05]
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
        // ����λ��
        particles[i].position += particles[i].velocity * deltaTime;

        // ��������
        particles[i].life -= deltaTime;

        if (particles[i].life <= 0.0) {
            // ������������
            particles[i].position = glm::vec3(
                static_cast<float>(rand()) / RAND_MAX,
                static_cast<float>(rand()) / RAND_MAX,
                static_cast<float>(rand()) / RAND_MAX
            );

            float angleXY = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846f; // ���XYƽ�淽��
            float speedXY = static_cast<float>(rand()) / RAND_MAX * 0.1f; // ���XYƽ���ٶȴ�С
            float speedZ = static_cast<float>(rand()) / RAND_MAX * 0.1f - 0.05f; // ���Z�����ٶȴ�С����Χ��[-0.05, 0.05]
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

    // ����ͶӰ����ͼ����� uniform
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // ���������
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ParticlebrushTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "brushTexture"), 0);

    //��������
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

    // �����������
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ����ͼ�����ݵ�������
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
            format = GL_RGB; // Ĭ�ϸ�ʽ

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::cerr << "Successfully loaded particle texture: " << filePath << std::endl;
    }
    else {
        std::cerr << "Failed to load texture at path: " << filePath << std::endl;
    }
    stbi_image_free(data);
}

