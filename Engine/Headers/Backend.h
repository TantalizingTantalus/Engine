#pragma once
#include "ImGuizmo.h"
#include "imgui/imgui-master/backends/imgui_impl_glfw.h"
#include "imgui/imgui-master/backends/imgui_impl_opengl3.h"
#include <windows.h>
#include <commdlg.h>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <typeinfo>
#include <type_traits>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "Camera.h"
#include "stb_image.h"
#include "FrameBuffer.h"

#include "imgui/imgui-master/imgui.h"
#include "imgui/imgui-master/imgui_internal.h"
#include "Logging.h"
#include "Light.h"
#include "SystemIcons.h"
#include <thread>
#include <future>

#include <filesystem>
#include "spdlog/spdlog.h"
#include <map>
#include "Util.h"
#include "ft2build.h"
#include "Editor.h"
#include "Component.h"

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
	bool m_FirstMouseSceneClick = false;
	float m_EditorSetting_FontSize = 24.0f;
	float lastX = 0, lastY = 0;
	float m_CurrentUI_FontSize = 48.0f;
	std::string fontPath = "Fonts/arial.ttf";
	std::string projectNamePath = "HelloWorld";
	int m_SceneWidth, m_SceneHeight;
	std::unordered_map<int, Entity> m_ModelMap;
	std::vector<Entity> m_LoadedModelsList;
	FT_Library ft;
	FT_Face face;
	Camera camera;
	Time EditorTime;
	GLFWwindow* m_Window;
	
	//std::vector<Entity> m_PointLights;
	std::unordered_map<int, Entity> m_PointLightsEntityMap;
	//std::mutex m_ModelMapMutex;
	DirectionalLight MyDirLight;

	Backend();
	~Backend();
	int Initialize();
	bool InitializeFreeType(const std::string& fontPath);
	void InitializeUserInterface();
	int Update();
	
	bool Run();
	bool RenderModels();
	bool RenderUI();
	bool StartImGui();
	bool LoadEngineIcon();
	bool UpdateDockingScene();
	bool UpdateFontSize(float);
	void HandleMouseClick(GLuint);
	void PollInputs(GLFWwindow* window);
	void PollMouseMovement(float, float);
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
	

	

	// Base instance of ModelList
	//std::vector<Entity> ModelList;
	int selectedDebugModelIndex = -1;

	
	

};