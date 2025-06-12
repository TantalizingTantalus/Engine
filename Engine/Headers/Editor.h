#pragma once
#include "Backend.h"
#include "Light.h"
#include "Material.h"
#include <thread>
#include "../physx/PxPhysicsAPI.h"


class Backend;


class Editor
{
public:
	bool editingName = false,
		renderUI = false,
		isCreatingFolder = false,
		DEBUG_MODE = true,
		DEBUG_NORMAL_MAP = false,
		IsFullscreen = false,
		m_SceneHovered = false,
		m_FileViewerHovered = false,
		m_DebugHovered = false,
		m_InspectorHovered = false,
		m_LoadNewLayout = false,
		m_DebugEditorTrackMouse = false,
		m_PanningScene = false,
		m_VsyncEnabledInEditor = false;
	;

	float OutlineThickness = 1.01f;
	double MouseX, MouseY;
	char InputName[30];
	int selectedDebugModelIndex;
	std::string editingNameLoggingMsg;
	std::string newFolderName = "New_Folder";
	std::string editingTempName;
	Backend* myBack = nullptr;
	GLFWwindow* window = nullptr;
	Camera* camera = nullptr;
	ImGuizmo::OPERATION EditorTransformationOperation = ImGuizmo::OPERATION::TRANSLATE;
	ImVec4 clear_color;
	//std::vector<Entity>* DebugEntityList;
	std::unordered_map<int, Entity>* m_DebugEntityMap;
	std::filesystem::path myPath;
	std::filesystem::path myNavWindowPath;
	std::filesystem::path selectedPath;
	std::mutex m_DebugEntityMapMutex;
	Entity* DebugSelectedEntity = nullptr;
	std::vector<std::string> LoggingEntries;
	ImVec2 m_SceneWindowPosition;
	ImVec2 m_SceneCursorPosition;
	
	Editor() : myPath(".."), myNavWindowPath("..")
	{
		spdlog::info("Initializing Editor Object...");
		clear_color = ImVec4(0.098f, 0.098f, 0.098f, 1.0f);
	}

	void Init(Backend&);
	void Task_Delete();
	void Task_FocusObject();
	bool Task_LoadDefaultLayout();
	void Task_ImportModel(std::unordered_map<int, Entity>&);
	void Task_DebugNormals(bool&, GLuint);
	void Exit_Application(GLFWwindow* window);
	void ToggleFullscreen(GLFWwindow* window, Backend*);
	void Toggle_UI();
	void RecursiveDisplayFolders(const std::filesystem::path&);
	void RecursiveDisplayChildren(const Entity&);
	void WindowUpdate(Camera& in_camera, GLFWwindow& in_window);
	void UpdateEntities(std::unordered_map<int, Entity>&);
	void PollEditorInput(GLFWwindow*);

	Model OpenModelFileDialog(std::unordered_map<int, Entity>&);
	void DebugWindow(ImGuiIO& io, std::unordered_map<int, Entity>&);

private:
	
	float minIconSize = 20.0f, maxIconSize = 100.0f, currentIconSize = 75.0f;
};