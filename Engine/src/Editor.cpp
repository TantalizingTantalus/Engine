#include "../Headers/Editor.h"
#include <cstring>

void Editor::Init(Backend& backend)
{

	myBack = &backend;
	myNavWindowPath = myBack->projectNamePath;
	myPath = myBack->projectNamePath;
}

void Editor::WindowUpdate(Camera& in_camera, GLFWwindow& in_window)
{
	this->camera = &in_camera;
	this->window = &in_window;

}

void Editor::PollEditorInput(GLFWwindow* window)
{
	if (!ImGui::IsAnyItemActive())
	{
		if (ImGui::IsKeyPressed(ImGuiKey_W))
		{
			EditorTransformationOperation = ImGuizmo::OPERATION::TRANSLATE;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_R))
		{
			EditorTransformationOperation = ImGuizmo::OPERATION::ROTATE;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_R) && ImGui::GetIO().KeyCtrl)
		{
			editingName = true;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_T))
		{
			EditorTransformationOperation = ImGuizmo::OPERATION::SCALE;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_H))
		{
			DEBUG_MODE = !DEBUG_MODE;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F11))
		{
			ToggleFullscreen(nullptr, myBack);
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F))
		{
			Task_FocusObject();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_N))
		{
			auto& model = DebugSelectedEntity->GetComponent<Model>();
			if (model.GetVisible())
			{
				model.RenderMode = (model.RenderMode == RENDERTARGETS::NORMAL) ? RENDERTARGETS::LIT : RENDERTARGETS::NORMAL;
			}
			else
			{
				LoggingEntries.push_back("No object with shader selected!!");
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_L))
		{
			if (DebugSelectedEntity)
			{
				auto& model = DebugSelectedEntity->GetComponent<Model>();
				if (model.GetVisible())
				{
					model.RenderMode = (model.RenderMode == RENDERTARGETS::LINES) ? RENDERTARGETS::LIT : RENDERTARGETS::LINES;
				}
			}
			else
			{
				LoggingEntries.push_back("No object selected!!");
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			Task_Delete();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			Exit_Application(nullptr);
		}

		if (ImGui::IsKeyPressed(ImGuiKey_O) && ImGui::GetIO().KeyCtrl)
		{
			Task_ImportModel(*m_DebugEntityMap);
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F12))
		{
			system("start https://learnopengl.com/");
		}

		if (ImGui::IsKeyPressed(ImGuiKey_X))
		{
			renderUI = !renderUI;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_M))
		{
			// camera.mode = static_cast<Camera_Mode>((camera.mode + 1) % 2);
		}
	}

	// Handle input while editing text
	if (ImGui::IsAnyItemActive())
	{
		
		if (ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			if (editingName)
			{
				editingNameLoggingMsg = fmt::format("{}", editingNameLoggingMsg);
				LoggingEntries.push_back(fmt::format("{} to \"{}\" ", editingNameLoggingMsg, editingTempName));
				editingNameLoggingMsg = "";
				DebugSelectedEntity->Name = editingTempName;
				editingName = false;
			}
		}
	}
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

void Editor::UpdateEntities(std::unordered_map<int, Entity>& EntityMap)
{
	std::lock_guard<std::mutex> lock(m_DebugEntityMapMutex);

	m_DebugEntityMap = &EntityMap;

}

void Editor::DebugWindow(ImGuiIO& io, std::unordered_map<int, Entity>& EntityMap)
{
	using namespace ImGui;
	if (BeginMenuBar())
	{
		if (BeginMenu("File"))
		{
			if (MenuItem("Import Model..", "Ctrl+O"))
			{
				{
					//std::async(std::launch::async, &Editor::Task_ImportModel, this, std::ref(EntityMap));
					Task_ImportModel(EntityMap);
				}
			}
			if (MenuItem("Close", "'Esc'")) { Exit_Application(window); }
			ImGui::EndMenu();
		}
		if (BeginMenu("Edit"))
		{
			//if (ImGui::MenuItem("Set Light Position to Camera", "'G'")) { Task_AlignDirLight(); }
			if (MenuItem("Focus Camera to Selected", "'F'")) { Task_FocusObject(); }
			if (MenuItem("Rename Selected Object", "'CTRL+R'")) { editingName = true; }
			ImGui::EndMenu();
		}
		if (BeginMenu("View"))
		{
			
			if (MenuItem("Hide Debug Window", "'H'")) { Toggle_UI(); }
			if (MenuItem("Hide/Show HUD", "'X'")) { renderUI = !renderUI; }
			if (MenuItem("Change Camera Mode", "'M'")) { camera->mode = static_cast<Camera_Mode>((camera->mode + 1) % 2); }
			if (MenuItem("Reset Window Layout", "'PG DN'")) { Task_LoadDefaultLayout(); }

			ImGui::EndMenu();
		}
		if (BeginMenu("Docs"))
		{
			if (MenuItem("OpenGL Docs", "'F12'")) { system("start https://learnopengl.com/"); }
			if (MenuItem("ImGui Docs", "'F12+1'")) { system("start https://github.com/ocornut/imgui/wiki"); }
			if (MenuItem("Guizmo Docs", "'F12+2'")) { system("start https://github.com/CedricGuillemet/ImGuizmo"); }
			if (MenuItem("GitHub Page", "'F12+3'")) { system("start https://github.com/TantalizingTantalus/Engine"); }
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

			if (ImGui::IsWindowHovered())
			{
				m_InspectorHovered = true;
			}
			else {
				m_InspectorHovered = false;
			}

			for (auto& ent : EntityMap)
			{
				glm::vec3 modelPosition = ent.second.transform->getLocalPosition();
				glm::vec3 modelScale = ent.second.transform->getLocalScale();
				glm::vec3 displayModelRotation = ent.second.transform->rotation;
				glm::vec3 modelRotation = ent.second.transform->getLocalRotation();
				if (ent.second.GetEntity() == DebugSelectedEntity)
				{
					DebugSelectedEntity = ent.second.GetEntity();

					// Display ALL contents of Components list
					ent.second.ShowComponents();

					// Debug quick info
					ImGui::Text(fmt::format("Entity ID: {}", DebugSelectedEntity->ID).c_str());
					Text(fmt::format("Scene Objects: {}", m_DebugEntityMap->size()).c_str());
				}
			}

			ImGui::End();
		}

		// Free look in the scene window
		{
			
			ImVec2 mousePos = ImGui::GetMousePos();

			// m_SceneCursor here is just the x, y of the mouse over the scene window specifically.
			// current window mouse position x and y minus the scene window position gives correct x and y offsets.
			m_SceneCursorPosition = ImVec2(mousePos.x - m_SceneWindowPosition.x, mousePos.y - m_SceneWindowPosition.y);
			
			if (m_DebugEditorTrackMouse)
			{
				if (Begin("Debug Track Mouse"))
				{
					ImGui::Text(fmt::format("Mouse X: {}   Mouse Y: {}", mousePos.x, mousePos.y).c_str());
					ImGui::Text(fmt::format("Window X: {}   Window Y: {}", m_SceneWindowPosition.x, m_SceneWindowPosition.y).c_str());
					ImGui::Text(fmt::format("Scene Mouse X: {}   Scene Mouse Y: {}", m_SceneCursorPosition.x, m_SceneCursorPosition.y).c_str());
					End();
				}
				
			}

			/*std::cout << m_SceneHovered << std::endl;*/
			if (m_SceneHovered)
			{
				ImGui::FocusItem();

				 
				if (ImGui::IsMouseClicked(1))
				{
					m_PanningScene = true;
					myBack->m_FirstMouseSceneClick = true;
				}


			}
			if (m_PanningScene)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				myBack->camera.SetFreeLook(true);
				myBack->PollInputs(myBack->m_Window);



				myBack->PollMouseMovement(m_SceneCursorPosition.x, m_SceneCursorPosition.y);
			}

			if (ImGui::IsMouseReleased(1))
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				myBack->camera.SetFreeLook(false);
				m_PanningScene = false;
			}

		}

		// Object selection/ object viewer 
		{
			ImGui::Begin("Object Viewer", nullptr);

			for (auto& ent : EntityMap)
			{
				bool isSelected = (ent.first == selectedDebugModelIndex);
				bool isOpened = false;
				if (!ent.second.children.empty())
				{
					isOpened = (ImGui::TreeNodeEx(ent.second.Name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow));
				}
				else
				{
					if (ent.second.parent == nullptr)
					{
						if (ImGui::Selectable(ent.second.Name.c_str()))
						{
							DebugSelectedEntity = ent.second.GetEntity();
							if (DebugSelectedEntity->HasComponent<Transform>())
							{
								camera->OrbitTarget = DebugSelectedEntity->GetComponent<Transform>().position;
							}
						}
					}
				}

				if (ImGui::IsItemClicked())
				{
					//selectedDebugModelIndex = i;
					DebugSelectedEntity = ent.second.GetEntity();
					camera->OrbitTarget = DebugSelectedEntity->GetComponent<Transform>().position;
				}

				if (isOpened)
				{
					RecursiveDisplayChildren(*ent.second.GetEntity());
					ImGui::TreePop();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
				{
					ImGui::OpenPopup("ContextMenu");
				}

				if (editingName && selectedDebugModelIndex == ent.first)
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
							Model newCube("Models/Light_Cube.fbx");
							Entity newCubeEntity;
							if (newCube.GetModelName() != "null_model")
							{
								// Transform Component
								newCubeEntity.transform->setLocalPosition(glm::vec3(0.0f));
								newCubeEntity.AddComponent(newCubeEntity.transform);

								// Model Component
								std::shared_ptr<Model> newCubeModelComp = std::make_shared<Model>(newCube);
								newCubeEntity.AddComponent(newCubeModelComp);

								// Material Component
								Material newMat(&newCubeEntity);
								std::shared_ptr<Material> myMatComp = std::make_shared<Material>(newMat);
								newCubeEntity.AddComponent(myMatComp);

								// Ship it
								newCubeEntity.ID = EntityMap.size() + 1;
								//EntityMap.emplace(newCubeEntity.ID, &newCubeEntity);
								myBack->SelectEntity(newCubeEntity.ID);
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
			
			ImVec2 DebugPos = ImGui::GetWindowPos();
			ImVec2 sceneSize = ImGui::GetWindowSize();

			ImVec2 mousePos = ImGui::GetIO().MousePos;

			if (ImGui::IsWindowHovered())
			{
				m_DebugHovered = true;
			}
			else
			{
				m_DebugHovered = false;
			}

			SeparatorText("Window Info");
			Text("Scene hovered: ");
			SameLine();
			Checkbox(" ", &m_SceneHovered);
			Text("File Viewer hovered: ");
			SameLine();
			Checkbox(" ", &m_FileViewerHovered);
			Text("Debug hovered: ");
			SameLine();
			Checkbox(" ", &m_DebugHovered);
			Text("Inspector hovered: ");
			SameLine();
			Checkbox(" ", &m_InspectorHovered);

			if (Button("Toggle Fullscreen Mode"))
				ToggleFullscreen(window, myBack);

			Checkbox("Track Mouse Position: ", &m_DebugEditorTrackMouse);

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

			// Object Border Outline
			SeparatorText("Outline Thickness");
			SliderFloat("##linethickness", &OutlineThickness, 1.0, 1.5);

			// UI Font Size
			SeparatorText("UI Font Size");
			if (SliderFloat("##UI_FontSize", &myBack->m_CurrentUI_FontSize, 1.0f, 72.0f))
			{
				if (!myBack->UpdateFontSize(myBack->m_CurrentUI_FontSize))
				{
					spdlog::warn(fmt::format("Unable to update font size to {}", myBack->m_CurrentUI_FontSize));
				}
			}

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
			char buffer[100]; 
			
			ImGui::Begin("File Viewer");


			if (ImGui::IsWindowHovered())
			{
				m_FileViewerHovered = true;
			}
			else
			{
				m_FileViewerHovered = false;
			}

			ImGui::Columns(2, "split", true);

			ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.2f);
			ImGui::SameLine();
			ImGui::Text(myBack->projectNamePath.c_str());
			ImGui::BeginChild("NavigationPanel");
			ImGui::Separator();
			ImGui::Spacing();
			if (!std::filesystem::exists(myNavWindowPath))
			{
				std::filesystem::create_directory(myNavWindowPath);
			}
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
			}
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::SliderFloat("##IconSizeSlider", &currentIconSize, minIconSize, maxIconSize, "%.2f");
			ImGui::PopItemWidth();

			ImGui::BeginChild("ChildFileViewer");
			
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

	
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
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
		LoggingEntries.push_back("Focused object");
		camera->LookAtWithYaw(DebugSelectedEntity->GetComponent<Transform>().position);
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
		if (!m_DebugEntityMap->empty())
		{
			m_DebugEntityMap->erase(DebugSelectedEntity->ID);
			DebugSelectedEntity = m_DebugEntityMap->begin()->second.GetEntity();
		}
	}
	else
	{
		LoggingEntries.push_back("No object selected!!");
	}
}

void Editor::Task_ImportModel(std::unordered_map<int, Entity>& EntityMap)
{
	std::filesystem::path originalWorkingDir = std::filesystem::current_path();

	Model newModel = OpenModelFileDialog(EntityMap);
	Entity newEntity(newModel.GetModelName().c_str());
	Material material(&newEntity);

	if (newEntity.Name != "null_model")
	{
		// create components
		std::shared_ptr<Model> modelComp = std::make_shared<Model>(newModel);
		std::shared_ptr<Material> matComp = std::make_shared<Material>(material);
		

		// add components
		newEntity.AddComponent(newEntity.transform); 
		newEntity.AddComponent(modelComp); 
		newEntity.AddComponent(matComp);

		// Initialize components
		matComp->Initialize(newEntity.GetComponent<Model>()); // bug bc newModel goes out of scope

		// Send it
		{
			newEntity.ID = EntityMap.size() + 1;
			std::lock_guard<std::mutex> lock(m_DebugEntityMapMutex);
			EntityMap.emplace(newEntity.ID, newEntity);
			myBack->SelectEntity(newEntity.ID);
		}
		//EntityMap.emplace(newEntity.ID, newEntity); 
	}

	std::filesystem::current_path(originalWorkingDir);
}

Model Editor::OpenModelFileDialog(std::unordered_map<int, Entity>& EntityMap)
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
