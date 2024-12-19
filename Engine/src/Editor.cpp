#include "../Headers/Editor.h"
#include <cstring>

void Editor::Init(Backend& backend)
{

	myBack = &backend;
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
			if (!child->children.empty())
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

void Editor::DebugWindow(ImGuiIO& io, std::vector<Entity>& ModelList)
{
	using namespace ImGui;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Import Model..", "Ctrl+O"))
			{
				Task_ImportModel(ModelList);
			}
			if (ImGui::MenuItem("Close", "'Esc'")) { Exit_Application(window); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			//if (ImGui::MenuItem("Set Light Position to Camera", "'G'")) { Task_AlignDirLight(); }
			if (ImGui::MenuItem("Focus Camera to Selected", "'F'")) { Task_FocusObject(); }
			if (ImGui::MenuItem("Rename Selected Object", "'CTRL+R'")) { editingName = true; }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Hide Debug Window", "'H'")) { Toggle_UI(); }
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

	if (DEBUG_MODE)
	{
		
		// Properties Panel
		{
			std::string PanelTitle = "Properties";

			ImGui::Begin(PanelTitle.c_str(), nullptr);

			for (int i = 0; i < ModelList.size(); i++)
			{
				Entity& model = ModelList[i];
				glm::vec3 modelPosition = model.transform->getLocalPosition();
				glm::vec3 modelScale = model.transform->getLocalScale();
				glm::vec3 displayModelRotation = model.transform->rotation;
				glm::vec3 modelRotation = model.transform->getLocalRotation();
				if (ModelList[i].GetEntity() == DebugSelectedEntity)
				{
					DebugSelectedEntity = model.GetEntity();

					// Display ALL contents of Components list
					model.ShowComponents();


					// Debug Buttons **** Ignore for now *****
					ImGui::SeparatorText("Debug Testing");
					ImGui::PushTextWrapPos(ImGui::GetWindowContentRegionMax().x);
					if (ImGui::Button("Parent", ImVec2((ImGui::GetWindowContentRegionMax().x * .8f) / 2.0f, 50)))
					{
						if (ModelList.size() > 2)
						{
							ModelList[1].AddChild(ModelList[2].GetEntity());
							
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Update MM", ImVec2((ImGui::GetWindowContentRegionMax().x * .8f) / 2.0f, 50)))
					{
						if (ModelList.size() > 2)
						{
							for (auto&& child : DebugSelectedEntity->children)
							{
								child->transform->m_modelMatrix = DebugSelectedEntity->transform->m_modelMatrix * child->transform->m_modelMatrix;
							}
						}
					}
					ImGui::PopTextWrapPos();
					ImGui::Text(fmt::format("Entity ID: {}", DebugSelectedEntity->ID).c_str());
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
							camera->OrbitTarget = DebugSelectedEntity->GetComponent<Transform>().position;
						}
					}
				}

				if (ImGui::IsItemClicked())
				{
					//selectedDebugModelIndex = i;
					DebugSelectedEntity = ModelList[i].GetEntity();
					camera->OrbitTarget = DebugSelectedEntity->GetComponent<Transform>().position;
				}


				if (isOpened)
				{
					RecursiveDisplayChildren(*ModelList[i].GetEntity());
					ImGui::TreePop();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
				{
					ImGui::OpenPopup("ContextMenu");
				}

				if (editingName && selectedDebugModelIndex == i)
				{
					char test[12];
					strcpy_s(test, DebugSelectedEntity->GetComponent<Model>().GetModelName().c_str());

					// add update messaage
					editingNameLoggingMsg = fmt::format("Updated model name from \"{}\"", DebugSelectedEntity->Name);

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
							Entity newCubeEntity;
							if (newCube.GetModelName() != "null_model")
							{
								newCubeEntity.transform->setLocalPosition(glm::vec3(0.0f));
								newCubeEntity.AddComponent(newCubeEntity.transform);
								ModelList.push_back(newCubeEntity);
								selectedDebugModelIndex = ModelList.size() - 1;
								DebugSelectedEntity = &ModelList[selectedDebugModelIndex];
							}
						}
						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Component")) {
						if (ImGui::MenuItem("Test Component 1")) {
							std::shared_ptr<TestComponent> TestC = std::make_shared<TestComponent>();
							TestC->compName = "hey-heypeople";
							DebugSelectedEntity->AddComponent(TestC);
							LoggingEntries.push_back(fmt::format("Added a new component to {}!", DebugSelectedEntity->Name));
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Test Component 2")) {
							std::shared_ptr<TestComponent> TestC = std::make_shared<TestComponent>();
							TestC->compName = "whats crackin'";
							DebugSelectedEntity->AddComponent(TestC);
							LoggingEntries.push_back(fmt::format("Added a new component to {}!", DebugSelectedEntity->Name));
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Transform")) {
							std::shared_ptr<Transform> TestT = std::make_shared<Transform>();

							DebugSelectedEntity->AddComponent(TestT);
							LoggingEntries.push_back(fmt::format("Added a new component to {}!", DebugSelectedEntity->Name));
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Light")) {
							std::shared_ptr<Light> TestT = std::make_shared<Light>();

							DebugSelectedEntity->AddComponent(TestT);
							LoggingEntries.push_back(fmt::format("Added a new component to {}!", DebugSelectedEntity->Name));
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Material")) {
							std::shared_ptr<Material> TestT = std::make_shared<Material>(DebugSelectedEntity);
							TestT->Initialize(DebugSelectedEntity->GetComponent<Model>());
							DebugSelectedEntity->AddComponent(TestT);
							LoggingEntries.push_back(fmt::format("Added a new component to {}!", DebugSelectedEntity->Name));
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

					//Model DuplicateItem(DebugSelectedEntity->GetModel().fullFilePath); // error due to ECS, needs DebugSelectedEntity
					//													 // instead of DebugSelectedObj
					//int numDupes = 0;
					//LoggingEntries.push_back("Keep in mind this duplicate is not instanced :(");
					//for (int i = 0; i < ModelList.size(); i++)
					//{
					//	if (ModelList[i].GetModelFileName() == DuplicateItem.GetModelFileName())
					//	{
					//		numDupes++;
					//	}
					//}
					//if (numDupes > 0)
					//	DuplicateItem.SetModelName(fmt::format("{}({})", DuplicateItem.GetModelName(), numDupes));
					//DuplicateItem.AddComponent(DuplicateItem.transform);
					//ModelList.push_back(DuplicateItem);
					///*selectedDebugModelIndex = ModelList.size() - 1;
					//DebugSelectedObj = &ModelList[selectedDebugModelIndex];*/
					//DebugSelectedEntity = ModelList[ModelList.size() - 1].GetEntity();

					LoggingEntries.push_back("Under Construction");
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

		// Debug properties 
		{
			Begin("Debug", nullptr);
			if (Button("Toggle Fullscreen Mode"))
				ToggleFullscreen(window, myBack);


			// Camera Position
			SeparatorText("Camera:");
			Text("X position: %.2f", camera->Position.x);
			Text("Y position: %.2f", camera->Position.y);
			Text("Z position: %.2f", camera->Position.z);

			// Camera Rotation
			Text("Yaw: %.2f", camera->Yaw);
			Text("Pitch: %.2f", camera->Pitch);

			// Field of view
			SeparatorText("Field of View (FOV):");
			SliderFloat("##fov", &camera->Zoom, 0, 100);

			// Zoom Scrolling
			SeparatorText("Zoom Scroll Factor: ");
			SliderFloat("##zoomScrollFac", &camera->m_ZoomScrollFactor, 0.1f, 10.0f);

			// Near clipping
			SeparatorText("Camera near clipping: ");
			InputFloat("##nearClipping", &camera->NearClippingPlane);
			// Far clipping
			SeparatorText("Camera far clipping: ");
			InputFloat("##farClipping", &camera->FarClippingPlane);
			// Adjust camera Speed
			SeparatorText("Camera Speed: ");
			SliderFloat("##CameraSpeed", &camera->MovementSpeed, camera->Min_MoveSpeed, camera->Max_MoveSpeed);

			SeparatorText("Outline Thickness");
			SliderFloat("##linethickness", &OutlineThickness, 1.0, 1.5);

			// Color picker
			SeparatorText("Directional Color");
			if (ColorEdit3("##directionalColor", (float*)&myBack->MyDirLight.m_Color, ImGuiColorEditFlags_PickerHueWheel))
			{
				myBack->MyDirLight.m_DirShader->setVec3("dirLight.color", glm::vec3(myBack->MyDirLight.m_Color.x, myBack->MyDirLight.m_Color.y, myBack->MyDirLight.m_Color.z));
			}
			SeparatorText("Directional Intensity");
			if (InputFloat("##DirIntensity", &myBack->MyDirLight.m_Intensity))
			{
				myBack->MyDirLight.m_DirShader->setFloat("dirLight.intensity", myBack->MyDirLight.m_Intensity);
			}
			SeparatorText("Viewport Color:");
			ColorEdit3("##viewportColor", (float*)&clear_color, ImGuiColorEditFlags_PickerHueWheel);

			// Version/Renderer info
			const GLubyte* glVersion = glGetString(GL_VERSION);
			const GLubyte* glRenderer = glGetString(GL_RENDERER);
			SeparatorText("Misc: ");
			Text("GPU: %s", glRenderer);
			Text("Application %.1f FPS", io.Framerate);
			Text("GLFW Version: %s", glfwGetVersionString());
			Text("OpenGL Version: %s", glVersion);
			End();
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
				std::filesystem::path tempPath(buffer);
				if (std::filesystem::exists(tempPath))
				{
					myNavWindowPath = tempPath;
				}
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
				if (ImGui::ImageButton("../", SystemIcons::GetBackButtonIcon(), ImVec2(currentIconSize, currentIconSize)))
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
							if (ImGui::ImageButton(s.c_str(),SystemIcons::GetFolderIcon(), ImVec2(currentIconSize, currentIconSize)))
							{
								myPath /= p_path.filename();
							}
							ImGui::Spacing();



							ImGui::TextWrapped(fmt::format("{}", fileNames.c_str()).c_str()); // may be a point of contention with const char* and 



						}
						else
						{
							fileNames = p_path.filename().string();
							ImGui::ImageButton(s.c_str(), SystemIcons::GetFileIcon(), ImVec2(currentIconSize, currentIconSize));
							ImGui::Spacing();
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);


							ImGui::TextWrapped(fmt::format("{}", fileNames.c_str()).c_str());

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

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	else {

		ImGui::Render();
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

// Quick hide function for debug ui elements
void Editor::Toggle_UI()
{
	DEBUG_MODE = !DEBUG_MODE;
}

bool Editor::Task_LoadDefaultLayout()
{
	// This loads the window layout from default file
	std::ifstream src("../Engine/DefaultLayout.ini", std::ios::binary);
	std::ofstream dst("../Engine/imgui.ini", std::ios::binary);

	std::filesystem::path defaultLayoutPath = "../Engine/DefaultLayout.ini";
	std::filesystem::path imGuiLayoutPath = "../Engine/imgui.ini";

	if (!std::filesystem::exists(defaultLayoutPath))
	{
		spdlog::error("Default ini could not be found, check if the directory exists.");

	}

	try
	{
		std::filesystem::copy_file(defaultLayoutPath, imGuiLayoutPath, std::filesystem::copy_options::overwrite_existing);

	}
	catch (const std::filesystem::filesystem_error& err)
	{
		spdlog::error(fmt::format("Ran into issues while loading defaultlayout.ini, error here:\n{}", err.what()));
	}
	


	return true;
}

void Editor::Exit_Application(GLFWwindow* window)
{
	std::cout << "Goodbye!" << std::endl;

	glfwSetWindowShouldClose(window, true);
}

void Editor::ToggleFullscreen(GLFWwindow* window, Backend* backObject)
{
	if (Editor::IsFullscreen)
	{
		Editor::IsFullscreen = !Editor::IsFullscreen;

		glfwSetWindowMonitor(window, glfwGetWindowMonitor(window), 100, 100, backObject->m_Width, backObject->m_Height, 0);
	}
	else
	{
		Editor::IsFullscreen = !Editor::IsFullscreen;
		glfwSetWindowMonitor(window, NULL, 0, 0, backObject->m_FullWidth, backObject->m_FullHeight, 0);
	}
}


void Editor::Task_FocusObject()
{
	if (DebugSelectedEntity == nullptr)
	{
		LoggingEntries.push_back("Debug Selected Object is empty (null)");
	}
	else
	{
		/*glm::vec3 ObjPosition = DebugSelectedEntity->transform->getLocalPosition();
		if (camera->Position != ObjPosition)
		{
			glm::vec3 ObjPosition = DebugSelectedEntity->transform->getLocalPosition();
			LookAtObject(ObjPosition);
			std::string message = std::format("Focused '{}' Object", DebugSelectedEntity->Name);
			LoggingEntries.push_back(message);
		}*/
		LoggingEntries.push_back("Focused object");
		
		//camera->m_ViewMatrix = glm::lookAt(camera->Position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		camera->LookAtWithYaw(DebugSelectedEntity->GetComponent<Transform>().position);
		//camera->updateCameraVectors();
	}
}

void Editor::Task_DebugNormals(bool& flag, GLuint sId)
{
	flag = !flag;

}

void Editor::Task_Delete()
{
	if (DebugSelectedEntity != nullptr)
	{
		if (!DebugEntityList->empty())
		{
			for (auto iModel = DebugEntityList->begin(); iModel != DebugEntityList->end(); )
			{
				if (iModel->Name == DebugSelectedEntity->Name)
				{
					iModel = DebugEntityList->erase(iModel);
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

void Editor::Task_ImportModel(std::vector<Entity>& ModelList)
{
	std::filesystem::path originalWorkingDir = std::filesystem::current_path();

	Model newModel = OpenModelFileDialog(ModelList);
	Entity newEntity(newModel.GetModelName().c_str());
	Material material(&newEntity);

	if (newEntity.Name != "null_model")
	{
		// create components
		std::shared_ptr<Model> modelComp = std::make_shared<Model>(newModel);
		std::shared_ptr<Material> matComp = std::make_shared<Material>(material);
		

		// add components
		newEntity.AddComponent(newEntity.transform); // Transform Component
		newEntity.AddComponent(modelComp); // Model Component
		newEntity.AddComponent(matComp); // Material Component

		// Initialize components
		matComp->Initialize(newEntity.GetComponent<Model>()); // bug bc newModel goes out of scope

		// Send it
		ModelList.push_back(newEntity); 
		DebugSelectedEntity = ModelList[ModelList.size() - 1].GetEntity();
		DebugSelectedEntity->ID = ModelList.size();
	}

	std::filesystem::current_path(originalWorkingDir);
}

Model Editor::OpenModelFileDialog(std::vector<Entity>& ModelList)
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
				if (ModelList[i].Name == extName || ModelList[i].Name == (fmt::format("{}({})", extName, i)))
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
