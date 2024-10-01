#pragma once
#include "../Headers/Backend.h"


class Editor
{
public:
	bool editingName = false, 
		renderUI = false, 
		DEBUG_MODE = true, 
		IsFullscreen = false
	;

	char InputName[30];
	int selectedDebugModelIndex;
	std::string editingNameLoggingMsg;
	std::string editingTempName;
	GLFWwindow* window;
	Camera* camera;
	ImGuizmo::OPERATION myOperation = ImGuizmo::OPERATION::TRANSLATE;
	Model* DirectionalLightObject = nullptr;
	Model* DebugSelectedObj = nullptr;
	ImVec4 clear_color = ImVec4(0.21f, 0.21f, 0.21f, .21f);
	std::vector<Model>* DebugModelList;
	std::vector<std::string> LoggingEntries;

	void Task_AlignDirLight();
	void Task_Delete();
	void LookAtObject(glm::vec3& ObjPosition);
	void Task_FocusObject();
	void Task_ImportModel(std::vector<Model>& ModelList);
	void Task_DebugNormals(bool&, GLuint);
	void Exit_Application(GLFWwindow* window);
	void ToggleFullscreen(GLFWwindow* window);
	void Hide_UI();
	void Show_UI();
	void Initialize(Camera& in_camera, GLFWwindow& in_window);

	Model OpenModelFileDialog(std::vector<Model>& ModelList);
	void DebugWindow(ImGuiIO& io, std::vector<Model>& ModelList);
};