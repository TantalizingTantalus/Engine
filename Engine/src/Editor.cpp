#include "../Headers/Editor.h"
#include <cstring>

void Editor::Init()
{
	folderIcon = (void*)(intptr_t)LoadFileIconID(fileFolderIconPath);
	fileIcon = (void*)(intptr_t)LoadFileIconID(fileFileIconPath);
	backButtonIcon = (void*)(intptr_t)LoadFileIconID(backButtonIconPath);
}

void Editor::WindowUpdate(Camera& in_camera, GLFWwindow& in_window)
{
	this->camera = &in_camera;
	this->window = &in_window;

	
}

void Editor::RecursiveDisplayChildren(const Entity& entity)
{
	if (entity.children.size() <= 0)
	{
		if (ImGui::Selectable(entity.Name.c_str())) {
			*DebugSelectedEntity = entity;
		}
	}
	else {
		for (const auto& child : entity.children) {
			if (child->children.size() <= 0) // problem here somewhere
			{
				if (ImGui::Selectable(child->Name.c_str()))
				{
					DebugSelectedEntity = child;
				}
			}
			else {
				if (ImGui::TreeNodeEx(child->Name.c_str())) {
					RecursiveDisplayChildren(*child);
					ImGui::TreePop();
				}
			}
		}
	}
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
			if (ImGui::MenuItem("Reset Window Layout", "'PG DN'")) { Task_LoadDefaultLayout(); }

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
		std::string PanelTitle = "Properties";
		
		ImGui::Begin(PanelTitle.c_str(), nullptr);

		for (int i = 0; i < ModelList.size(); i++)
		{
			Model& model = ModelList[i];
			glm::vec3 modelPosition = model.transform->getLocalPosition();
			glm::vec3 modelScale = model.transform->getLocalScale();
			glm::vec3 displayModelRotation = model.transform->rotation;
			glm::vec3 modelRotation = model.transform->getLocalRotation();
			if (ModelList[i].GetEntity() == DebugSelectedEntity)
			{
				DebugSelectedObj = &model;
				DebugSelectedEntity = model.GetEntity();

				// Display ALL contents of Components list
				model.ShowComponents();


				// Debug Buttons **** Ignore for now *****
				if (ImGui::Button("Make Parent"))
				{
					if (ModelList.size() > 2)
					{
						ModelList[1].AddChild(ModelList[2].GetEntity());
						//ModelList[1].UpdateSelfAndChild();
					}
				}

				if (ImGui::Button("Update Matrix"))
				{
					if (ModelList.size() > 2)
					{
						for (auto&& child : DebugSelectedObj->children)
						{
							child->transform->m_modelMatrix = DebugSelectedObj->transform->m_modelMatrix * child->transform->m_modelMatrix;
						}
					}
				}
			}
		}

		ImGui::End();
	}

	// Object selection/ object viewer 
	{
		ImGui::Begin("Object Viewer", nullptr);
		for (int i = 0; i < ModelList.size(); i++)
		{
			bool isSelected = (i == selectedDebugModelIndex);
			bool isOpened = false;
			if (!ModelList[i].children.empty())
			{
				isOpened = (ImGui::TreeNodeEx(ModelList[i].Name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow));
			}
			else
			{
				if (ModelList[i].parent == nullptr)
				{
					if (ImGui::Selectable(ModelList[i].Name.c_str()))
					{
						DebugSelectedEntity = ModelList[i].GetEntity();
					}
				}
			}

			if (ImGui::IsItemClicked())
			{
				//selectedDebugModelIndex = i;
				DebugSelectedEntity = ModelList[i].GetEntity();
			}


			if (isOpened)
			{
				RecursiveDisplayChildren(*ModelList[i].GetEntity());
				ImGui::TreePop();
			}

			// Selectable for testing purposes - delete later
			//if (ImGui::Selectable(ModelList[i].GetModelName().c_str(), isSelected))
			//{
			//	// to do 
			//	selectedDebugModelIndex = i;
			//	LoggingEntries.push_back(fmt::format("{} Selected", ModelList[i].GetModelName().c_str()));
			//}

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
			if (ImGui::BeginMenu("Add")) {
				if (ImGui::BeginMenu("Object")) {
					if (ImGui::MenuItem("Cube")) {
						Model newCube("../Engine/Models/Light_Cube.fbx");
						if (newCube.GetModelName() != "null_model")
						{
							newCube.transform->setLocalPosition(glm::vec3(0.0f));
							newCube.AddComponent(newCube.transform);
							ModelList.push_back(newCube);
							selectedDebugModelIndex = ModelList.size() - 1;
							DebugSelectedObj = &ModelList[selectedDebugModelIndex];
						}
					}
					ImGui::EndMenu();  
				}

				if (ImGui::BeginMenu("Component")) {
					if (ImGui::MenuItem("Test Component 1")) {
						std::shared_ptr<TestComponent> TestC = std::make_unique<TestComponent>();
						TestC->compName = "hey-heypeople";
						DebugSelectedObj->AddComponent(TestC);
						LoggingEntries.push_back("Added a new component!");
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Test Component 2")) {
						std::shared_ptr<TestComponent> TestC = std::make_unique<TestComponent>();
						TestC->compName = "whats crackin'";
						DebugSelectedObj->AddComponent(TestC);
						LoggingEntries.push_back("Added a new component!");
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Transform")) {
						std::shared_ptr<Transform> TestT = std::make_unique<Transform>();
						
						DebugSelectedObj->AddComponent(TestT);
						LoggingEntries.push_back("Added a new component!");
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Light")) {
						std::shared_ptr<Light> TestT = std::make_unique<Light>();

						DebugSelectedObj->AddComponent(TestT);
						LoggingEntries.push_back("Added a new component!");
					}

					ImGui::EndMenu();  
				}

				ImGui::EndMenu();  
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Rename")) {
				

				editingName = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Duplicate")) {
				
				Model DuplicateItem(DebugSelectedObj->fullFilePath);
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
				DuplicateItem.AddComponent(DuplicateItem.transform);
				ModelList.push_back(DuplicateItem);
				selectedDebugModelIndex = ModelList.size() - 1;
				DebugSelectedObj = &ModelList[selectedDebugModelIndex];
			}
			ImGui::Separator();
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
		ImGui::Begin("Debug", nullptr);
		if (ImGui::Button("Toggle Fullscreen Mode"))
			ToggleFullscreen(window);


		// Camera Position
		ImGui::SeparatorText("Camera:");
		ImGui::Text("X position: %.2f", camera->Position.x);
		ImGui::Text("Y position: %.2f", camera->Position.y);
		ImGui::Text("Z position: %.2f", camera->Position.z);

		// Camera Rotation
		ImGui::Text("Yaw: %.2f", camera->Yaw);
		ImGui::Text("Pitch: %.2f", camera->Pitch);

		// Field of view
		ImGui::SeparatorText("Field of View (FOV):");
		ImGui::SliderFloat("##fov", &camera->Zoom, 0, 100);
		// Near clipping
		ImGui::SeparatorText("Camera near clipping: ");
		ImGui::InputFloat("##nearClipping", &camera->NearClippingPlane);
		// Far clipping
		ImGui::SeparatorText("Camera far clipping: ");
		ImGui::InputFloat("##farClipping", &camera->FarClippingPlane);
		// Adjust camera Speed
		ImGui::SeparatorText("Camera Speed: ");
		ImGui::SliderFloat("##CameraSpeed", &camera->MovementSpeed, camera->Min_MoveSpeed, camera->Max_MoveSpeed);


		// Color picker
		ImGui::SeparatorText("Viewport Color:");
		ImGui::ColorEdit3("##viewportColor", (float*)&clear_color, ImGuiColorEditFlags_Float);

		// Version/Renderer info
		const GLubyte* glVersion = glGetString(GL_VERSION);
		const GLubyte* glRenderer = glGetString(GL_RENDERER);
		ImGui::SeparatorText("Misc: ");
		ImGui::Text("GPU: %s", glRenderer);
		ImGui::Text("Application %.1f FPS", io.Framerate);
		ImGui::Text("GLFW Version: %s", glfwGetVersionString());
		ImGui::Text("OpenGL Version: %s", glVersion);
		ImGui::End();
	}

	// Logging window! a personal favorite
	{
		ImGui::Begin("Logging", nullptr);
		ImGui::Text("Logging window successfully initialized!");
		
		for (int i = 0; i < LoggingEntries.size(); i++)
		{
			ImGui::Text(LoggingEntries[i].c_str());
		}
		ImGui::End();
	}

	// To do implement content file browser
	{
		char buffer[100]; // 100 can be reduced if needed

		ImGui::Begin("File Viewer");

		ImGui::Columns(2, "split", true);

		
		ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.2f); 
		ImGui::SameLine();
		ImGui::Text("Project Files");
		ImGui::BeginChild("NavigationPanel");
		ImGui::Separator();
		ImGui::Spacing();
		for (auto& p : std::filesystem::directory_iterator(myNavWindowPath))
		{
			if (p.is_directory())
			{
				RecursiveDisplayFolders(p.path());
			}
		}
		ImGui::Spacing();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::Text("Directory:");
		ImGui::SameLine();
		strcpy_s(buffer, myPath.string().c_str());
		if (ImGui::InputText("##WorkingDirectoryInput", buffer, IM_ARRAYSIZE(buffer)))
		{
			// to do:
			//  make editable text input
		}
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x); 
		ImGui::SliderFloat("##IconSizeSlider", &currentIconSize, minIconSize, maxIconSize, "%.2f");
		ImGui::PopItemWidth();

		ImGui::BeginChild("ChildFileViewer");
		
		if (ImGui::BeginTable("##FileViewTable", 8))
		{
			float spacing = 5.0f;
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
			ImGui::TableNextColumn();
			if (ImGui::ImageButton("../", backButtonIcon, ImVec2(currentIconSize, currentIconSize)))
			{
				// Go up a directory
				if (myPath.has_parent_path())
				{
					myPath = myPath.parent_path();
				}
			}
			ImGui::TableNextColumn();

			if (std::filesystem::exists(myPath))
			{
				for (auto& p : std::filesystem::directory_iterator(myPath))
				{
					auto& p_path = p.path();
					std::string fileNames;
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
					std::string s = p_path.filename().string();
					if (p.is_directory())
					{
						fileNames = p_path.filename().string();
						if (ImGui::ImageButton(s.c_str(), folderIcon, ImVec2(currentIconSize, currentIconSize)))
						{
							myPath /= p_path.filename();
						}
						ImGui::Spacing();

						ImGui::Text(fmt::format("{}", fileNames.c_str()).c_str()); // may be a point of contention with const char* and 

					}
					else
					{
						fileNames = p_path.filename().string();
						ImGui::ImageButton(s.c_str(), fileIcon, ImVec2(currentIconSize, currentIconSize));
						ImGui::Spacing();
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
						ImGui::Text(fmt::format("{}", fileNames.c_str()).c_str());
					}


					ImGui::TableNextColumn();
				}
			}
			else
			{
				spdlog::error("Directory {} does not exist!", myPath.string());
			}

			ImGui::EndTable();
		}
		
		

		ImGui::EndChild();
		
		ImGui::End();
	}

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

void Editor::RecursiveDisplayFolders(const std::filesystem::path& directoryPath)
{
	if (!std::filesystem::is_directory(directoryPath))
	{
		return;
	}

	bool isOpened = ImGui::TreeNodeEx(directoryPath.filename().string().c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth);

	if (ImGui::IsItemClicked())
	{
		myPath = directoryPath;
	}

	if (isOpened)
	{

		for (auto& entry : std::filesystem::directory_iterator(directoryPath))
		{
			if (entry.is_directory())
			{

				RecursiveDisplayFolders(entry.path());
			}
		}

		ImGui::TreePop();
	}
	
}

GLuint Editor::LoadFileIconID(const char* path)
{
	GLuint textureID;
	int height, width, channels;

	unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
	if (data == nullptr) {
		std::cerr << "Error loading icon for reason: " << stbi_failure_reason() << std::endl;
		return 0; 
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Texture params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data); 
	return textureID;
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

bool Editor::Task_LoadDefaultLayout()
{
	std::ifstream src("DefaultLayout.ini", std::ios::binary);
	std::ofstream dst("imgui.ini", std::ios::binary);

	if (src && dst)
	{
		dst << src.rdbuf();
	}
	else
	{
		std::cerr << "Error loading layout";
		return false;
	}
	// Reload the layout settings from imgui.ini
	ImGui::LoadIniSettingsFromDisk("imgui.ini");  

	// Close all currently open windows (this resets the layout to the new one)
	ImGuiContext& g = *ImGui::GetCurrentContext();
	for (int i = 0; i < g.Windows.Size; i++) {
		ImGui::CloseCurrentPopup(); 
	}

	
	return true;
}

void Editor::Task_AlignDirLight()
{
	LoggingEntries.push_back("Under Construction...");

	DirectionalLightObject->transform->setLocalPosition(camera->Position);
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
		glm::vec3 ObjPosition = DebugSelectedObj->transform->getLocalPosition();
		if (camera->Position != ObjPosition)
		{
			glm::vec3 ObjPosition = DebugSelectedObj->transform->getLocalPosition();
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
	std::filesystem::path originalWorkingDir = std::filesystem::current_path();
	
	Model newModel = OpenModelFileDialog(ModelList);
	if (newModel.GetModelName() != "null_model")
	{
		ModelList.push_back(newModel);
		DebugSelectedEntity = ModelList[ModelList.size() - 1].GetEntity();
	}

	std::filesystem::current_path(originalWorkingDir);
}

Model Editor::OpenModelFileDialog(std::vector<Model>& ModelList)
{																			
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
		loadedModel.AddComponent(loadedModel.transform);

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
