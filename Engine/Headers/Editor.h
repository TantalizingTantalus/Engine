#pragma once
#include "Backend.h"
#include "Light.h"
#include "Material.h"
#include "../physx/PxPhysicsAPI.h"


class Backend;


class Editor
{
public:
	bool editingName = false,
		renderUI = false,
		DEBUG_MODE = true,
		DEBUG_NORMAL_MAP = false,
		IsFullscreen = false,
		m_SceneHovered = false,
		m_LoadNewLayout = false;
	;
	float OutlineThickness = 1.01f;
	char InputName[30];
	int selectedDebugModelIndex;
	std::string editingNameLoggingMsg;
	std::string editingTempName;
	Backend* myBack = nullptr;
	GLFWwindow* window = nullptr;
	Camera* camera = nullptr;
	ImGuizmo::OPERATION myOperation = ImGuizmo::OPERATION::TRANSLATE;
	ImVec4 clear_color;
	std::vector<Entity>* DebugEntityList;
	Entity* DebugSelectedEntity = nullptr;
	std::vector<std::string> LoggingEntries;
	
	Editor() : myPath(".."), myNavWindowPath("..")
	{
		spdlog::info("Initializing Editor Object...");
		clear_color = ImVec4(0.098f, 0.098f, 0.098f, 1.0f);
	}

	void Init(Backend&);
	void Task_Delete();
	void Task_FocusObject();
	bool Task_LoadDefaultLayout();
	void Task_ImportModel(std::vector<Entity>& ModelList);
	void Task_DebugNormals(bool&, GLuint);
	void Exit_Application(GLFWwindow* window);
	void ToggleFullscreen(GLFWwindow* window, Backend*);
	void Toggle_UI();
	void RecursiveDisplayFolders(const std::filesystem::path&);
	void RecursiveDisplayChildren(const Entity&);
	void WindowUpdate(Camera& in_camera, GLFWwindow& in_window);

	Model OpenModelFileDialog(std::vector<Entity>& ModelList);
	void DebugWindow(ImGuiIO& io, std::vector<Entity>& ModelList);

private:
	std::filesystem::path myPath;
	std::filesystem::path myNavWindowPath;
	float minIconSize = 20.0f, maxIconSize = 100.0f, currentIconSize = 75.0f;
};