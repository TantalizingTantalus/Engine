

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
    glm::vec3 OrbitTarget;
    glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_ViewMatrix;
    // custom properties

    Camera_Mode mode = Camera_Mode::PERSPECTIVE;

    // euler Angles
    float Yaw = 0.0f;
    float Pitch = 0.0f;

    // Default camera values
    static constexpr float YAW = -90.0f;
    static constexpr float PITCH = 0.0f;
    static constexpr float SPEED = 2.5f;
    static constexpr float SENSITIVITY = 0.1f;
    static constexpr float ZOOM = 87.0f;

    // camera options
    float NearClippingPlane = .1f;
    float FarClippingPlane = 100.0f;

    float MovementSpeed;
    float BoostSpeed = 5.0f;
    float m_ZoomScrollFactor = 7.2f;
   
    const float Max_MoveSpeed = 10.0f;
    const float Min_MoveSpeed = 0.0f;
    float MouseSensitivity;
    float Zoom;

    int m_WindowWidth, m_WindowHeight;

    bool isOrbiting = false;
    bool freeLook = false;



    float GetDistance(glm::vec3 Source, glm::vec3 Target)
    {
        float distance =  sqrt((Source.x - Target.x) * (Source.x - Target.x) +
            (Source.y - Target.y) * (Source.y - Target.y) +
            (Source.z - Target.z) * (Source.z - Target.z));

        
        return distance;
    }

    glm::mat4 LookAtMatrix(glm::vec3 position, glm::vec3 target)
    {
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 zaxis = glm::normalize(position - target); 
        glm::vec3 xaxis = glm::normalize(glm::cross(worldUp, zaxis)); 
        glm::vec3 yaxis = glm::cross(zaxis, xaxis); 

        glm::mat4 orientation = glm::mat4(1.0f);
        orientation[0][0] = xaxis.x;
        orientation[1][0] = xaxis.y;
        orientation[2][0] = xaxis.z;
        orientation[0][1] = yaxis.x;
        orientation[1][1] = yaxis.y;
        orientation[2][1] = yaxis.z;
        orientation[0][2] = zaxis.x;
        orientation[1][2] = zaxis.y;
        orientation[2][2] = zaxis.z;

        glm::mat4 translation = glm::mat4(1.0f);
        translation[3][0] = -position.x;
        translation[3][1] = -position.y;
        translation[3][2] = -position.z;

        return orientation * translation;
    }

  

    

    void LookAtWithYaw(glm::vec3 target) {
        // Get direction to the target
        glm::vec3 direction = glm::normalize(target - Position);

        // get yaw based on the direction
        Yaw = atan2(direction.z, direction.x) * 180.0f / glm::pi<float>();
        Pitch = atan2(direction.y, sqrt(direction.x * direction.x + direction.z * direction.z)) * 180.0f / glm::pi<float>();

        Pitch = glm::clamp(Pitch, -89.0f, 89.0f);

        // Ensure yaw is between -180 and 180
        if (Yaw < -180.0f) Yaw += 360.0f;
        if (Yaw > 180.0f) Yaw -= 360.0f;

        // Update camera vectors
        updateCameraVectors();
    }

    
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;

        updateCameraVectors();
    }
    
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

    void OrbitAroundTarget(glm::vec3 target, float radius, const float sensitivity, float deltaYaw, float deltaPitch)
    {
        Yaw -= deltaYaw * sensitivity;
        Pitch += deltaPitch * sensitivity;
        deltaYaw -= Yaw * sensitivity;
        deltaPitch += Pitch * sensitivity;

        if (Pitch > glm::radians(89.0f)) {
            Pitch = glm::radians(89.0f);
        }
        if (Pitch < glm::radians(-89.0f)) {
            Pitch = glm::radians(-89.0f);
        }

        float camX = static_cast<float>(sin(glfwGetTime()) * radius );
        //float camY = target.y + radius * sin(Pitch);  
        float camZ = static_cast<float>(cos(glfwGetTime()) * radius);

        Position = glm::vec3(camX, Position.y, camZ);

        LookAtWithYaw(target);
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;


        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void UpdateViewAndProjectionMatrices()
    {
        if (this->mode == Camera_Mode::PERSPECTIVE)
            this->m_ProjectionMatrix = glm::perspective(glm::radians(Zoom), (float)m_WindowWidth / (float)m_WindowHeight, NearClippingPlane, FarClippingPlane);
        else if (this->mode == Camera_Mode::ORTHO)
        {
            float aspectRatio = (float)m_WindowWidth / (float)m_WindowHeight;
            float orthoHeight = Zoom / 10.0f;
            float orthoWidth = orthoHeight * aspectRatio;
            this->m_ProjectionMatrix = glm::ortho(
                -orthoWidth / 2.0f,
                orthoWidth / 2.0f,
                -orthoHeight / 2.0f,
                orthoHeight / 2.0f ,
                NearClippingPlane,
                FarClippingPlane
            );
        }
        this->m_ViewMatrix = glm::lookAt(Position, Position + Front, Up);
    }

    
    void ProcessMouseScroll(float yoffset)
    {
        // To do implement zoom via positioning instead of cam FOV

        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 120.0f)
            Zoom = 120.0f;
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
};

