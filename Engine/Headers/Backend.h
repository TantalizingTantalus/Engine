#pragma once
#include "ImGuizmo.h"
#include "imgui/imgui-master/backends/imgui_impl_glfw.h"
#include "imgui/imgui-master/backends/imgui_impl_opengl3.h"
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

#include "imgui/imgui-master/imgui.h"
#include "imgui/imgui-master/imgui_internal.h"
#include "../Headers/Logging.h"
#include "../Headers/Light.h"
#include "../Headers/SystemIcons.h"


#include <filesystem>
#include "spdlog/spdlog.h"
#include <map>
#include "../Headers/Util.h"
#include "ft2build.h"
#include "../Headers/Editor.h"
#include "../Headers/Component.h"

#include FT_FREETYPE_H

#include <iostream>

struct DirectionalLight
{

	Shader* m_DirShader = nullptr;
	
	glm::vec3 m_Direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	ImVec4 m_Color = ImVec4(59.0f / 255.0f, 1.0f, 103.0f / 255.0f, 1.0f);
	float m_Intensity = 1.0f;
	bool isActive = true;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

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
	static const int m_Height = 900, m_Width = 1400;
	int m_FullHeight = 1080, m_FullWidth = 1920;
	float m_EditorSetting_FontSize = 24.0f;
	std::string fontPath = "../Engine/Fonts/arial.ttf";
	int m_SceneWidth, m_SceneHeight;
	
	std::vector<Entity> m_PointLights;
	DirectionalLight MyDirLight;

	Backend();
	int Initialize();
	void InitializeUserInterface();
	int Update();
	
	bool Run();
	bool RenderModels();
	bool RenderUI();
	bool StartImGui();
	bool LoadEngineIcon();
	bool UpdateDockingScene();
	void HandleMouseClick(GLuint);
	void SelectEntity(int id);
	int GetWindowWidth(GLFWwindow* window) { int height, width; glfwGetWindowSize(window, &width, &height); return width; }
	int GetWindowHeight(GLFWwindow* window) { int height, width; glfwGetWindowSize(window, &width, &height); return height; }
	Backend& GetBackEnd() { return *this; }
private:
	
	float money = 0.0f;
	
	Shader m_LitMaterialShader; 
	Shader m_LightShader;
	Shader m_TextShader;
	Shader m_StencilShader;
	Shader m_ShadowShader;
	

	GLFWwindow* m_Window;

	// Base instance of ModelList
	std::vector<Entity> ModelList;
	int selectedDebugModelIndex = -1;

};