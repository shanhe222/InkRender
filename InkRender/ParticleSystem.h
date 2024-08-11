// ParticleSystem.h
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

struct Particle {
    glm::vec3 position, velocity;
    glm::vec4 color;
    float life;
    float pressure;
    float angle; // 用于笔触方向

    Particle() : position(0.0f), velocity(0.0f, 1.0f, 0.0f), color(1.0f), life(1.0f) { }
};

class ParticleSystem {
public:
    ParticleSystem(int maxParticles, unsigned int shaderProgram);
    ~ParticleSystem();

    GLuint ParticlebrushTexture;

    void init();
    void update(float deltaTime,
        const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model);
    void render(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model);
    void loadTexture(const std::string& filePath);
    void setParticlesFromMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals);

private:
    std::vector<Particle> particles;
    int maxParticles;
    unsigned int shaderProgram;
    unsigned int VAO, VBO;
    

    void initBuffers();
};

