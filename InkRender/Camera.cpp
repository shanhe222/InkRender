#include "Camera.h"

// 构造函数
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// 返回观察矩阵
glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

// 处理输入的键盘事件
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime, bool isSpacePressed) {
    float velocity = MovementSpeed * deltaTime;
    if (isSpacePressed) {
        velocity *= 10.0f; // 如果空格键被按下，速度增加10倍
    }
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

// 处理输入的鼠标移动事件
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // 确保当俯仰角超过89度时，屏幕不会翻转
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // 更新前、右、上向量
    updateCameraVectors();
}

// 处理输入的鼠标滚轮事件
void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

// 重新计算摄像头的前、右、上向量
void Camera::updateCameraVectors() {
    // 计算新的前向量
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // 重新计算右向量和上向量
    Right = glm::normalize(glm::cross(Front, WorldUp));  // 保证向量的正交性
    Up = glm::normalize(glm::cross(Right, Front));
}
