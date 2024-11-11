#pragma once
#include "../Headers/Backend.h"
#include "../Headers/Light.h"

class Backend;


class Editor
{
public:
	bool editingName = false,
		renderUI = false,
		DEBUG_MODE = true,
		DEBUG_NORMAL_MAP = false,
		IsFullscreen = false
	;

	char InputName[30];
	int selectedDebugModelIndex;
	std::string editingNameLoggingMsg;
	std::string editingTempName;
	Backend* myBack = nullptr;
	GLFWwindow* window = nullptr;
	Camera* camera = nullptr;
	ImGuizmo::OPERATION myOperation = ImGuizmo::OPERATION::TRANSLATE;
	Model* DirectionalLightObject = nullptr;
	Model* DebugSelectedObj = nullptr;
	ImVec4 clear_color;
	std::vector<Model>* DebugModelList;
	std::vector<Entity>* DebugEntityList;
	Entity* DebugSelectedEntity = nullptr;
	std::vector<std::string> LoggingEntries;
	
	Editor() : myPath(".."), myNavWindowPath("..")
	{
		spdlog::info("Initializing Editor Object...");
		clear_color = ImVec4(0.098f, 0.098f, 0.098f, 1.0f);
	}

	void Init(Backend&);
	void Task_AlignDirLight();
	void Task_Delete();
	void LookAtObject(glm::vec3& ObjPosition);
	void Task_FocusObject();
	bool Task_LoadDefaultLayout();
	void Task_ImportModel(std::vector<Model>& ModelList);
	void Task_DebugNormals(bool&, GLuint);
	void Exit_Application(GLFWwindow* window);
	void ToggleFullscreen(GLFWwindow* window, Backend*);
	void Hide_UI();
	void Show_UI();
	void RecursiveDisplayFolders(const std::filesystem::path&);
	void RecursiveDisplayChildren(const Entity&);
	void WindowUpdate(Camera& in_camera, GLFWwindow& in_window);
	GLuint LoadFileIconID(const char* path);

	Model OpenModelFileDialog(std::vector<Model>& ModelList);
	void DebugWindow(ImGuiIO& io, std::vector<Model>& ModelList);

private:
	std::filesystem::path myPath;
	std::filesystem::path myNavWindowPath;
	const char* fileFolderIconPath = "C:\\Users\\Gaevi\\OneDrive\\Documents\\Extracted Icons\\4.png";
	const char* fileFileIconPath = "C:\\Users\\Gaevi\\OneDrive\\Documents\\Extracted Icons\\1.png";
	const char* backButtonIconPath = "C:\\Users\\Gaevi\\OneDrive\\Documents\\Extracted Icons\\UpArrow.png";
	ImTextureID folderIcon, fileIcon, backButtonIcon;
	float minIconSize = 20.0f, maxIconSize = 100.0f, currentIconSize = 75.0f;
};