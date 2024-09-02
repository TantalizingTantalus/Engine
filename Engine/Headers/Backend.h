#pragma once
#include "ImGuizmo.h"
#include "imgui-master/backends/imgui_impl_glfw.h"
#include "imgui-master/backends/imgui_impl_opengl3.h"
#include <windows.h>
#include <commdlg.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include "../Headers/Shader.h"
#include <glm/gtc/type_ptr.hpp>

#include "../Headers/Model.h"
#include "../Headers/Camera.h"
#include "../Headers/stb_image.h"
#include "imgui-master/imgui.h"
#include "imgui-master/imgui_internal.h"

#include <filesystem>
#include "spdlog/spdlog.h"
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void ToggleFullscreen(GLFWwindow* window);

class Backend
{
public:
	static const int height = 900, width = 1400;
	static const int full_height = 1440, full_width = 2560;
	static bool IsFullscreen;
	static glm::mat4 projection;
	static glm::mat4 view;
	Backend();
	int Initialize();
	int Update();
	int GetWindowWidth(GLFWwindow* window) { int height, width; glfwGetWindowSize(window, &width, &height); return width; }
	int GetWindowHeight(GLFWwindow* window) { int height, width; glfwGetWindowSize(window, &width, &height); return height; }
	Model OpenModelFileDialog();
private:
	void DebugWindow(ImGuiIO& io);
	
	float lastFrame = 0.0f;
	float rotationAngle = 0.0f;
	
	ImVec4 clear_color = ImVec4(0.21f, 0.21f, 0.21f, .21f);
	
	Shader TempShader; 
	Shader lightCubeShader;

	GLFWwindow* window;
	std::vector<Model> ModelList;
	int selectedDebugModelIndex = -1;
	bool testbool1 = false;
};