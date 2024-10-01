#include "../Headers/Editor.h"



void Editor::Initialize(Camera& in_camera, GLFWwindow& in_window)
{
	this->camera = &in_camera;
	this->window = &in_window;
}

void Editor::DebugWindow(ImGuiIO& io, std::vector<Model>& ModelList)
{

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O"))
			{
				Task_ImportModel(ModelList);

			}
			if (ImGui::MenuItem("Close", "'Esc'")) { Exit_Application(window); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Set Light Position to Camera", "'G'")) { Task_AlignDirLight(); }
			if (ImGui::MenuItem("Focus Camera to Selected", "'F'")) { Task_FocusObject(); }
			if (ImGui::MenuItem("Rename Selected Object", "'CTRL+R'")) { editingName = true; }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Hide Debug Window", "'H'")) { Hide_UI(); }
			if (ImGui::MenuItem("Hide/Show HUD", "'X'")) { renderUI = !renderUI; }
			if (ImGui::MenuItem("Change Camera Mode", "'M'")) { camera->mode = static_cast<Camera_Mode>((camera->mode + 1) % 2); }

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Docs"))
		{
			if (ImGui::MenuItem("OpenGL Docs", "'F12'")) { system("start https://learnopengl.com/"); }
			if (ImGui::MenuItem("ImGui Docs", "'F12+1'")) { system("start https://github.com/ocornut/imgui/wiki"); }
			if (ImGui::MenuItem("Guizmo Docs", "'F12+2'")) { system("start https://github.com/CedricGuillemet/ImGuizmo"); }
			if (ImGui::MenuItem("GitHub Page", "'F12+3'")) { system("start https://github.com/TantalizingTantalus/Engine"); }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();

	// Properties Panel
	{
		ImGui::Begin("Properties");

		for (int i = 0; i < ModelList.size(); i++)
		{
			Model& model = ModelList[i];
			glm::vec3& modelPosition = model.GetPosition();
			glm::vec3& modelScale = model.GetScale();
			glm::vec3& modelRotation = model.GetRotation();
			if (i == selectedDebugModelIndex)
			{
				ImGui::Text("Model Name: ");
				strcpy_s(InputName, model.GetModelName().c_str());
				ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackAlways;
				if (ImGui::InputText("##something", InputName, IM_ARRAYSIZE(InputName)))
				{
					DebugSelectedObj->SetModelName(InputName);
				}

				ImGui::Text("Directional Light?");
				ImGui::Checkbox("True/False", &model.IsLight);

				DebugSelectedObj = &model;
				ImGui::Text("Position");
				if (ImGui::InputFloat3("##position", glm::value_ptr(modelPosition))) {
					model.UpdateModelMatrix();
				}

				ImGui::Text("Rotation");
				if (ImGui::InputFloat3("##rotation", glm::value_ptr(modelRotation))) {
					model.UpdateModelMatrix();
				}

				ImGui::Text("Scale");
				if (ImGui::InputFloat3("##scale", glm::value_ptr(modelScale))) {
					model.UpdateModelMatrix();
				}
			}
		}

		ImGui::End();
	}

	// Object selection/ object viewer 
	{
		ImGui::Begin("Object Viewer");
		for (int i = 0; i < ModelList.size(); i++)
		{
			bool isSelected = (i == selectedDebugModelIndex);

			if (ImGui::Selectable(ModelList[i].GetModelName().c_str(), isSelected))
			{
				selectedDebugModelIndex = i; // Update the selected index
				LoggingEntries.push_back(fmt::format("{} Selected", ModelList[i].GetModelName().c_str()));
			}

			// Check if the item is hovered and the right mouse button is released
			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
			{
				ImGui::OpenPopup("ContextMenu");
			}


			if (editingName && selectedDebugModelIndex == i)
			{
				char test[12];
				strcpy_s(test, DebugSelectedObj->GetModelName().c_str());

				// add update messaage
				editingNameLoggingMsg = fmt::format("Updated model name from \"{}\"", DebugSelectedObj->GetModelName());

				if (ImGui::InputText("##something", test, IM_ARRAYSIZE(test)))
				{
					editingTempName = test;
				}
			}
		}

		if (ImGui::BeginPopup("ContextMenu")) {
			if (ImGui::MenuItem("Rename")) {
				// Perform action for Option 1

				editingName = true;
			}
			if (ImGui::MenuItem("Duplicate")) {
				// Perform action for Option 2
				Model DuplicateItem = *DebugSelectedObj;
				int numDupes = 0;
				LoggingEntries.push_back("Keep in mind this duplicate is not instanced :(");
				for (int i = 0; i < ModelList.size(); i++)
				{
					if (ModelList[i].GetModelFileName() == DuplicateItem.GetModelFileName())
					{
						numDupes++;
					}
				}
				if (numDupes > 0)
					DuplicateItem.SetModelName(fmt::format("{}({})", DuplicateItem.GetModelName(), numDupes));
				ModelList.push_back(DuplicateItem);
			}
			if (ImGui::MenuItem("Delete"))
			{
				Task_Delete();
			}

			ImGui::EndPopup();
		}

		ImGui::AlignTextToFramePadding();
		ImGui::End();
	}

	// Debug properties - to do cleanup
	{
		ImGui::Begin("Debug");
		if (ImGui::Button("Toggle Fullscreen Mode"))
			ToggleFullscreen(window);


		// Camera Position
		ImGui::Text("Camera:");
		ImGui::Text("X position: %.2f", camera->Position.x);
		ImGui::Text("Y position: %.2f", camera->Position.y);
		ImGui::Text("Z position: %.2f", camera->Position.z);

		// Camera Rotation
		ImGui::Text("Yaw: %.2f", camera->Yaw);
		ImGui::Text("Pitch: %.2f", camera->Pitch);

		// Field of view
		ImGui::Text("Field of View (FOV):");
		ImGui::SameLine();
		ImGui::SliderFloat("##fov", &camera->Zoom, 0, 100);

		// Near clipping
		ImGui::Text("Camera near clipping: ");
		ImGui::SameLine();
		ImGui::InputFloat("##nearClipping", &camera->NearClippingPlane);

		// Far clipping
		ImGui::Text("Camera far clipping: ");
		ImGui::SameLine();
		ImGui::InputFloat("##farClipping", &camera->FarClippingPlane);

		// Adjust camera Speed
		ImGui::Text("Camera Speed: ");
		ImGui::SameLine();
		ImGui::SliderFloat("Camera Speed", &camera->MovementSpeed, camera->Min_MoveSpeed, camera->Max_MoveSpeed);



		// Color picker
		ImGui::Text("Viewport Color:");
		ImGui::SameLine();
		ImGui::ColorEdit3("##viewportColor", (float*)&clear_color);

		// Version/Renderer info
		const GLubyte* glVersion = glGetString(GL_VERSION);
		const GLubyte* glRenderer = glGetString(GL_RENDERER);
		ImGui::Text("GPU: %s", glRenderer);
		ImGui::Text("Application %.1f FPS", io.Framerate);
		ImGui::Text("GLFW Version: %s", glfwGetVersionString());
		ImGui::Text("OpenGL Version: %s", glVersion);
		ImGui::End();
	}

	// Logging window! a personal favorite
	{
		ImGui::Begin("Logging");
		ImGui::Text("Logging window successfully initialized!");
		for (int i = 0; i < LoggingEntries.size(); i++)
		{
			ImGui::Text(LoggingEntries[i].c_str());
		}
		ImGui::End();
	}

	// To do implement content file browser
	//{
	//	ImGui::Begin("File Viewer");
	//	//ImGui::AcceptDragDropPayload("hello");
	//	
	//	ImGui::End();
	//}

	// Prepare render to draw
	ImGui::Render();

	// Draw it if Debug mode true - todo fix this, bug in program where entire window stalls when false, 
	// this is mostly likely due to the openGL renderer being rendered to a 'scene' window inside of the 
	// debug menu.
	if (DEBUG_MODE)
	{
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

// Quick toggle function for debug mode
void Editor::Show_UI()
{
	DEBUG_MODE = true;
}

// Quick hide function for debug ui elements
void Editor::Hide_UI()
{
	DEBUG_MODE = false;
}

void Editor::Task_AlignDirLight()
{
	/*LoggingWindowEntries.push_back(fmt::format("Updated light position to \nx:{}\ny:{}\nz:{}", camera.Position.x * roundDecimal, camera.Position.y * roundDecimal, camera.Position.z * roundDecimal));*/
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2)
		<< "Updated light position to \nx:" << camera->Position.x
		<< "\ny:" << camera->Position.y
		<< "\nz:" << camera->Position.z;

	LoggingEntries.push_back(oss.str());
	DirectionalLightObject->SetPosition(camera->Position);
	DirectionalLightObject->UpdateModelMatrix();
}

void Editor::Exit_Application(GLFWwindow* window)
{
	std::cout << "Goodbye!" << std::endl;

	glfwSetWindowShouldClose(window, true);
}

void Editor::ToggleFullscreen(GLFWwindow* window)
{
	if (Editor::IsFullscreen)
	{
		Editor::IsFullscreen = !Editor::IsFullscreen;
		spdlog::info(Editor::IsFullscreen);
		glfwSetWindowMonitor(window, NULL, 100, 100, Backend::width, Backend::height, 0);
		camera->Zoom = 45.0f;
	}
	else {
		Editor::IsFullscreen = !Editor::IsFullscreen;
		spdlog::info(Editor::IsFullscreen);
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 100, 100, Backend::full_width, Backend::full_height, 0);
		camera->Zoom = 90.0f;
	}
}

void Editor::LookAtObject(glm::vec3& ObjPosition)
{

	glm::vec3 zoffset = glm::vec3(0, 0, 3.0f);

	if (camera->Position != (ObjPosition - zoffset))
	{
		camera->Position = glm::vec3(ObjPosition.x, ObjPosition.y, ObjPosition.z - zoffset.z);
		camera->LookAtWithYaw(ObjPosition);
		camera->Pitch = -15.0f;
		camera->updateCameraVectors();
	}
}

void Editor::Task_FocusObject()
{
	if (DebugSelectedObj == nullptr)
	{
		LoggingEntries.push_back("Debug Selected Object is empty (null)");
	}
	else
	{
		glm::vec3 ObjPosition = DebugSelectedObj->GetPosition();
		if (camera->Position != ObjPosition)
		{
			glm::vec3 ObjPosition = DebugSelectedObj->GetPosition();
			LookAtObject(ObjPosition);
			std::string message = std::format("Focused '{}' Object", DebugSelectedObj->GetModelName());
			LoggingEntries.push_back(message);
		}
	}
}

void Editor::Task_DebugNormals(bool& flag, GLuint sId)
{
	flag = !flag;
	glUniform1i(glGetUniformLocation(sId, "DEBUG_NORMAL"), flag);
}

void Editor::Task_Delete()
{
	if (DebugSelectedObj != nullptr)
	{
		if (!DebugModelList->empty())
		{
			for (auto iModel = DebugModelList->begin(); iModel != DebugModelList->end(); )
			{
				if (iModel->GetModelName() == DebugSelectedObj->GetModelName())
				{
					iModel = DebugModelList->erase(iModel);
				}
				else
				{
					++iModel;
				}
			}
		}
	}
	else
	{
		LoggingEntries.push_back("No object selected!!");
	}
}

void Editor::Task_ImportModel(std::vector<Model>& ModelList)
{
	Model newModel = OpenModelFileDialog(ModelList);
	if (newModel.GetModelName() != "null_model")
	{
		ModelList.push_back(newModel);
	}
}

Model Editor::OpenModelFileDialog(std::vector<Model>& ModelList)
{																					// To Do: solve cancellation logic

	// Buffer to hold the file name
	wchar_t fileName[MAX_PATH] = L"";
	std::string logMsg;

	// Initialize OPENFILENAME structure
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;  // Window handles go here, not applicable atm
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName) / sizeof(fileName[0]);  // Adjust for wide characters
	ofn.lpstrFilter = L"Any(.fbx, .obj, .dae)\0*.fbx;*.obj;*.dae\0.obj\0*.obj\0.fbx\0*.fbx\0.dae\0*.dae\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box
	if (GetOpenFileName(&ofn) == TRUE)
	{
		std::string ofnName = Util::WideStringToString(ofn.lpstrFile);
		std::filesystem::path path(ofnName);
		std::string extName = path.filename().string();

		std::string baseFileName = extName;

		// extTemp example value: ".obj", ".fbx"
		std::string extTemp = extName.substr(extName.length() - 4);

		// extName example value: "Backpack", "Cube"
		extName = extName.substr(0, extName.length() - 4);

		//Check for duplicates
		{
			int numDupes = 0;
			for (int i = 0; i < ModelList.size(); i++)
			{
				if (ModelList[i].GetModelFileName() == baseFileName)
				{
					numDupes++;
				}
			}
			if (numDupes > 0)
				extName = fmt::format("{}({})", extName, numDupes);
		}


		logMsg = fmt::format("Successfully loaded file: {}{}", extName, extTemp);
		spdlog::info(logMsg);
		LoggingEntries.push_back(logMsg);

		// Load Model
		Model loadedModel(ofnName);
		loadedModel.SetModelName(extName);
		loadedModel.SetModelFileName(baseFileName);
		return loadedModel;
	}
	else
	{
		logMsg = "Open file selection cancelled";
		spdlog::info(logMsg);
		LoggingEntries.push_back(logMsg);
		Model model;
		return model;
	}
}
