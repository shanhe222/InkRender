#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// 定义摄像头移动的可能选项
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 摄像头默认值
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    // 摄像头属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // 欧拉角
    float Yaw;
    float Pitch;
    // 摄像头选项
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // 构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // 返回观察矩阵
    glm::mat4 GetViewMatrix();

    // 处理输入的键盘事件
    void ProcessKeyboard(Camera_Movement direction, float deltaTime, bool isSpacePressed);

    // 处理输入的鼠标移动事件
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // 处理输入的鼠标滚轮事件
    void ProcessMouseScroll(float yoffset);

private:
    // 重新计算摄像头的前、右、上向量
    void updateCameraVectors();
};

#endif
