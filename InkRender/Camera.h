#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// ��������ͷ�ƶ��Ŀ���ѡ��
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// ����ͷĬ��ֵ
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    // ����ͷ����
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // ŷ����
    float Yaw;
    float Pitch;
    // ����ͷѡ��
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // ���캯��
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // ���ع۲����
    glm::mat4 GetViewMatrix();

    // ��������ļ����¼�
    void ProcessKeyboard(Camera_Movement direction, float deltaTime, bool isSpacePressed);

    // �������������ƶ��¼�
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // ����������������¼�
    void ProcessMouseScroll(float yoffset);

private:
    // ���¼�������ͷ��ǰ���ҡ�������
    void updateCameraVectors();
};

#endif
