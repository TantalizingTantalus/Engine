#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../Headers/Shader.h"
#include "../Headers/RenderPipeline.h"
#include <glm/gtc/type_ptr.hpp>

#include "../Headers/ModelManager.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void PollInputs(GLFWwindow* window);

//ModelManager ModManager;
RenderPipeline Renderer;

int height = 600, width = 1000;
float rotationAngle = 0.0f;
std::vector<RObject*> cubes;

int main()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		std::cout << "GLFW failed initialization..." << std::endl;
		return 0;
	}
	
	// Create GLFW Window 
	GLFWwindow* window = glfwCreateWindow(800, 600, "Engine", NULL, NULL);
	if (!window)
	{
		std::cout << "Uh oh something went wrong..." << std::endl;
		return 0;
	}
	
	// Set the current context to the openGL window
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);



	// Load GL Libraries
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Initialize Shaders
	Shader shaders("..\\Engine\\Shaders\\vertex_shader.vert", "..\\Engine\\Shaders\\fragment_shader.frag");

	unsigned int cubeVAO = RObject::QuickGenerateCubeVAO(); 

	//ModelManager::LoadModel("C:\\Users\Gaevi\OneDrive\Documents\CodePractice\Engine\Engine\Textures");

	// How many cubes + offset
	int maxCubes = 1;
	for (int i = 0; i < maxCubes; ++i)
	{
		float offset = 0;
		auto* cube = new RObject(cubeVAO, &shaders);
		cube->SetPosition(glm::vec3((offset + i) * .5f, 0.0f, 0.0f)); 
		cubes.push_back(cube);
	}

	// Add cubes to the renderer with custom starting rotation
	for (auto* cube : cubes)
	{
		cube->SetRotation(glm::vec3(0.0f, 0.0f, 45.0f));
		cube->SetRotation(glm::vec3(-45.0f, 0.0f, 45.0f));
		cube->SetScale(glm::vec3(1.0f, .75f, .75f));
		Renderer.AddObject(cube);
	}

	Renderer.Initialize();

	
	
	// Main Loop *CORE*
	while (!glfwWindowShouldClose(window))
	{
		RenderPipeline::BeginFrame();
		RenderPipeline::DrawDebugAxes(1.0f);

		PollInputs(window);
		
		  
		rotationAngle += 0.5f;
		if (rotationAngle > 360.0f)
		{
			rotationAngle -= 360.0f; 
		}
		//spin baby
		for (auto& cube : cubes)
		{
			cube->SetRotation(glm::vec3(45.0f, rotationAngle, 90.0f));
		}

		Renderer.RenderBackground();
		Renderer.Draw();

		
		
		RenderPipeline::EndFrame(window);
		
	}
	
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void PollInputs(GLFWwindow* window)
{
	glfwSetKeyCallback(window, Input_Callback);
}

void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_W:
		if (action == GLFW_PRESS)
		{
			for (auto& cube : cubes)
			{
				glm::vec3 cubePos = cube->GetPosition();
				cube->SetPosition(glm::vec3(cubePos.x, cubePos.y + .15f, cubePos.z));

			}
		}
		break;

	case GLFW_KEY_D:
		if (action == GLFW_PRESS)
		{
			for (auto& cube : cubes)
			{
				glm::vec3 cubePos = cube->GetPosition();
				cube->SetPosition(glm::vec3(cubePos.x + .15f, cubePos.y, cubePos.z));
				
			}
		}
		break;

	case GLFW_KEY_A:
		if (action == GLFW_PRESS)
		{
			for (auto& cube : cubes)
			{
				glm::vec3 cubePos = cube->GetPosition();
				cube->SetPosition(glm::vec3(cubePos.x - .15f, cubePos.y, cubePos.z));

			}
		}
		break;

	case GLFW_KEY_S:
		if (action == GLFW_PRESS)
		{
			for (auto& cube : cubes)
			{
				glm::vec3 cubePos = cube->GetPosition();
				cube->SetPosition(glm::vec3(cubePos.x, cubePos.y - .15f, cubePos.z));

			}
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

