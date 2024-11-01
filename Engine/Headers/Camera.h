

#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Headers/Backend.h"
#include "../Headers/Util.h"

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

enum Camera_Mode {
    PERSPECTIVE,
    ORTHO
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 87.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::mat4 m_ProjectionMatrix;
    Camera_Mode mode = Camera_Mode::PERSPECTIVE;
    // euler Angles
    float Yaw;
    float Pitch;

    // camera options
    float NearClippingPlane = .1f;
    float FarClippingPlane = 100.0f;

    float MovementSpeed;
    float BoostSpeed = 5.0f;
    const float Max_MoveSpeed = 10.0f;
    const float Min_MoveSpeed = 0.0f;
    float MouseSensitivity;
    float Zoom;

    int m_WindowWidth, m_WindowHeight;


    void LookAtWithYaw(glm::vec3 target) {
        // Get direction to the target
        glm::vec3 direction = glm::normalize(target - Position);

        // get yaw based on the direction
        Yaw = atan2(direction.z, direction.x) * 180.0f / glm::pi<float>();
        Pitch = atan2(direction.y, sqrt(direction.x * direction.x + direction.z * direction.z)) * 180.0f / glm::pi<float>();

        // Ensure yaw is between -180 and 180
        if (Yaw < -180.0f) Yaw += 360.0f;
        if (Yaw > 180.0f) Yaw -= 360.0f;

        // Update camera vectors
        updateCameraVectors();
    }

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;

        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    void Initialize(GLFWwindow* InWindow)
    {
        glfwGetFramebufferSize(InWindow, &m_WindowWidth, &m_WindowHeight);
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return m_ViewMatrix;
    }

    glm::mat4 GetProjectionMatrix()
    {
        return m_ProjectionMatrix;
    }

    void SetFreeLook(bool flag)
    {
        this->freeLook = flag;
    }

    bool GetFreeLook() const { return this->freeLook; }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    void UpdateViewAndProjectionMatrices()
    {
        if (this->mode == Camera_Mode::PERSPECTIVE)
            this->m_ProjectionMatrix = glm::perspective(glm::radians(Zoom), (float)m_WindowWidth / (float)m_WindowHeight, NearClippingPlane, FarClippingPlane);
        else if (this->mode == Camera_Mode::ORTHO)
            this->m_ProjectionMatrix = glm::ortho(0.0f, 4.0f, 0.0f, 3.0f, 0.1f, 100.0f);
        this->m_ViewMatrix = glm::lookAt(Position, Position + Front, Up);
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 75.0f)
            Zoom = 75.0f;
    }

    float GetMovementSpeed() const { return MovementSpeed; }
    void SetMovementSpeed(float update) { if (update <= Max_MoveSpeed)MovementSpeed = update; else { MovementSpeed = Max_MoveSpeed; } }


    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));

        m_ViewMatrix = glm::lookAt(Position, Position + Front, Up);
    }
private:
    // custom properties
    bool freeLook = false;

    glm::mat4 m_ViewMatrix;
    
    
    // calculates the front vector from the Camera's (updated) Euler Angles
    
};

