#include "Camera.h"

// ���캯��
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// ���ع۲����
glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

// ��������ļ����¼�
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime, bool isSpacePressed) {
    float velocity = MovementSpeed * deltaTime;
    if (isSpacePressed) {
        velocity *= 10.0f; // ����ո�������£��ٶ�����10��
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

// �������������ƶ��¼�
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // ȷ���������ǳ���89��ʱ����Ļ���ᷭת
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // ����ǰ���ҡ�������
    updateCameraVectors();
}

// ����������������¼�
void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

// ���¼�������ͷ��ǰ���ҡ�������
void Camera::updateCameraVectors() {
    // �����µ�ǰ����
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // ���¼�����������������
    Right = glm::normalize(glm::cross(Front, WorldUp));  // ��֤������������
    Up = glm::normalize(glm::cross(Right, Front));
}
