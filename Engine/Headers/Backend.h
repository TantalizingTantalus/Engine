#pragma once
#include "ImGuizmo.h"
#include "imgui-master/backends/imgui_impl_glfw.h"
#include "imgui-master/backends/imgui_impl_opengl3.h"
#include <windows.h>
#include <commdlg.h>
#include <memory>
#include <typeinfo>
#include <type_traits>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include "../Headers/Shader.h"
#include <glm/gtc/type_ptr.hpp>

#include "../Headers/Model.h"
#include "../Headers/Camera.h"
#include "../Headers/stb_image.h"
#include "../Headers/FrameBuffer.h"

#include "imgui-master/imgui.h"
#include "imgui-master/imgui_internal.h"
#include "../Headers/Logging.h"

#include <filesystem>
#include "spdlog/spdlog.h"
#include <map>
#include "../Headers/Util.h"
#include "ft2build.h"
#include "../Headers/Editor.h"

#include FT_FREETYPE_H

#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

struct Time
{
public:
	float currentFrame;
	float deltaTime;
	float lastFrame = 0.0f;
	float currentTime;
	void Update()
	{
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		currentTime += deltaTime;
	}
};

class Backend
{
public:
	static const int height = 900, width = 1400;
	static const int full_height = 1440, full_width = 2560;
	int SceneWidth, SceneHeight;

	
	Backend();
	int Initialize();
	int Update();
	
	bool Run();
	bool RenderModels();
	bool RenderUI();
	bool StartImGui();
	bool LoadEngineIcon();
	bool UpdateDockingScene();
	int GetWindowWidth(GLFWwindow* window) { int height, width; glfwGetWindowSize(window, &width, &height); return width; }
	int GetWindowHeight(GLFWwindow* window) { int height, width; glfwGetWindowSize(window, &width, &height); return height; }
private:
	
	float rotationAngle = 0.0f;
	float money = 0.0f;
	
	Shader TempShader; 
	Shader lightCubeShader;
	Shader textShader;
	FrameBuffer sceneBuf;

	GLFWwindow* window;

	// Base instance of ModelList
	std::vector<Model> ModelList;
	int selectedDebugModelIndex = -1;
};