#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
  public:
    glm::vec3 position;
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 25.0f, 40.0f),
           glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw = -90.0f,
           float startPitch = -25.0f)
        : position(startPos), worldUp(startUp), yaw(startYaw), pitch(startPitch),
          movementSpeed(20.0f), mouseSensitivity(0.12f) {
        updateVectors();
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    void processKeyboard(CameraMovement direction, float deltaTime, bool fast) {
        float velocity = movementSpeed * deltaTime * (fast ? 2.5f : 1.0f);
        switch (direction) {
            case CameraMovement::FORWARD:  position += front * velocity; break;
            case CameraMovement::BACKWARD: position -= front * velocity; break;
            case CameraMovement::LEFT:     position -= right * velocity; break;
            case CameraMovement::RIGHT:    position += right * velocity; break;
            case CameraMovement::UP:       position += worldUp * velocity; break;
            case CameraMovement::DOWN:     position -= worldUp * velocity; break;
        }
    }

    void processMouseMovement(float xOffset, float yOffset) {
        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateVectors();
    }

  private:
    void updateVectors() {
        glm::vec3 newFront;
        newFront.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        newFront.y = std::sin(glm::radians(pitch));
        newFront.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        front = glm::normalize(newFront);

        right = glm::normalize(glm::cross(front, worldUp));
        up    = glm::normalize(glm::cross(right, front));
    }
};

