#pragma once
#include "imgui-master/imgui.h"
#include "imgui-master/backends/imgui_impl_glfw.h"
#include "imgui-master/backends/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include "../Headers/Shader.h"
#include <glm/gtc/type_ptr.hpp>

#include "../Headers/Model.h"
#include "../Headers/Camera.h"
#include "../Headers/stb_image.h"

#include "spdlog/spdlog.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


class Backend
{
public:
	Backend();
	int Initialize();
	int Update();
private:
	void DebugWindow(ImGuiIO& io);
	int full_height = 1440, full_width = 2560;
	int height = 900, width = 1400;
	float lastFrame = 0.0f;
	float rotationAngle = 0.0f;
	
	ImVec4 clear_color = ImVec4(0.21f, 0.21f, 0.21f, .21f);
	glm::mat4 projection;
	glm::mat4 view;
	Shader TempShader; 

	GLFWwindow* window;
	std::vector<Model> ModelList;
	bool testbool1 = false;
};