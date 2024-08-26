#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include "../Headers/Shader.h"
#include <glm/gtc/type_ptr.hpp>

#include "../Headers/ModelManager.h"
#include "../Headers/Model.h"
#include "../Headers/Camera.h"
#include "../Headers/stb_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void PollInputs(GLFWwindow* window);

int full_height = 1440, full_width = 2560;
int height = 900, width = 1400;
float deltaTime = 0.0f, lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
glm::mat4 view = camera.GetViewMatrix();
std::vector<Model> ModelList;

float rotationAngle = 0.0f;


int main()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		std::cout << "GLFW failed initialization..." << std::endl;
		return 0;
	}
	
	// Create GLFW Window 
	GLFWwindow* window = glfwCreateWindow(width, height, "Engine", NULL, NULL);
	if (!window)
	{
		std::cout << "Uh oh something went wrong..." << std::endl;
		return 0;
	}
	
	// Set the current context to the openGL window
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Lock cursor to window
	if (camera.GetFreeLook())
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwSetCursorPosCallback(window, mouse_callback);

	// Load GL Libraries
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);

	glEnable(GL_DEPTH_TEST);

	// Initialize Shaders
	Shader shaders("..\\Engine\\Shaders\\vertex_shader.vert", "..\\Engine\\Shaders\\fragment_shader.frag");


	shaders.use();

	// Load 3 models to render
	Model ourModel("../Engine/Models/Blockert.fbx");
	Model ourModel1("../Engine/Models/Backpack.obj");
	Model MonkeyMan("../Engine/Models/Monkey.obj");
	ModelList.push_back(ourModel);
	ModelList.push_back(ourModel1);
	ModelList.push_back(MonkeyMan);

	

	
	// Main Loop *CORE*
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); // Start Frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PollInputs(window);

		glClearColor(.50f, 0.5f, .5f, 1.0f);
		
		float currentFrame = glfwGetTime();  // Get the current time
		deltaTime = currentFrame - lastFrame; // Calculate delta time
		lastFrame = currentFrame;  // Update last frame time
		

		rotationAngle += 0.5f;
		if (rotationAngle > 360.0f)
		{
			rotationAngle -= 360.0f; 
		}

		projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		shaders.setMat4("projection", projection);
		shaders.setMat4("view", view);

		//Renderer.Draw();

		for (int i = 0; i < ModelList.size(); i++)
		{
			Model& modelitem = ModelList[i];
			modelitem.SetPosition(glm::vec3(-i + .5f, 0.0f, 0.0f));
			modelitem.SetRotation(-rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			modelitem.SetScale(glm::vec3(.25f, .25f, .25f));
			modelitem.Draw(shaders);
		}
		
		glfwSwapBuffers(window); // End Frame
		
	}
	
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void PollInputs(GLFWwindow* window)
{
	if (camera.GetFreeLook())
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);
	}

	glfwSetKeyCallback(window, Input_Callback);

}

void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (camera.GetFreeLook())
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	switch (key)
	{
	case GLFW_KEY_W:
		if (action == GLFW_PRESS)
		{
			/*model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
			std::cout << "somewhere around here" << std::endl;
			*/	
		}
		break;

	case GLFW_KEY_H:
		if (action == GLFW_PRESS)
		{
			camera.SetFreeLook(!camera.GetFreeLook());
		}

		
		break;
	case GLFW_KEY_G:
		if (action == GLFW_PRESS)
		{
			std::cout << "Camera position: \nx: " << camera.Position.x << "\ny: " << camera.Position.y << "\nz: " << camera.Position.z << std::endl;
		}


		break;

	case GLFW_KEY_A:
		if (action == GLFW_PRESS)
		{
			
		}
		break;

	case GLFW_KEY_S:
		if (action == GLFW_PRESS)
		{
			
		}
		break;

	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS)
		{
			std::cout << "Goodbye!" << std::endl;

			glfwSetWindowShouldClose(window, true);
		}
		break;
	}

	
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	static float lastX = 400, lastY = 300;
	static bool firstMouse = true;

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	if(camera.GetFreeLook())
		camera.ProcessMouseMovement(xoffset, yoffset);
}