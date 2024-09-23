#include "../Headers/Backend.h"


//																							Backend Global variables 
bool DEBUG_MODE = true;
bool DEBUG_NORMAL_MAP = false;
bool ContextMenuActive = false;
bool shouldSpin = true;
bool reverseSpin = false;
bool editingName = false;
float deltaTime;
char InputName[30];
Camera camera;
ImGuizmo::OPERATION myOperation = ImGuizmo::OPERATION::TRANSLATE;
Model* DirectionalLightObject = nullptr;
Model* DebugSelectedObj = nullptr;
std::string editingTempName, editingNameLoggingMsg;

// Session logging vector
std::vector<std::string> LoggingWindowEntries;
std::vector<Model>* DebugModelList;

//																								 Function protos




void Task_AlignDirLight();
void Task_Delete();
void LookAtObject(glm::vec3& ObjPosition);
void Task_FocusObject();
void Task_DebugNormals(bool&, GLuint);
void Exit_Application(GLFWwindow* window);
void Hide_UI();
void Show_UI();
void ToggleFullscreen(GLFWwindow* window);
void PollInputs(GLFWwindow* window);



//																									Main Logic
int Backend::Initialize()
{
	deltaTime = 0.0f;
	shouldSpin = true;
	Backend::IsFullscreen = false;
	camera.Position = (glm::vec3(0.0f, 0.0f, 3.0f));
	spdlog::info("Initializing GLFW");
	

	// Initialize GLFW
	if (!glfwInit())
	{
		spdlog::error("GLFW failed intialization...");
		return 0;
	}

	if (Backend::IsFullscreen)
	{
		window = glfwCreateWindow(full_width, full_height, "Engine", glfwGetPrimaryMonitor(), NULL);
		if (!window)
		{
			camera.Zoom = 95;
			spdlog::error("Uh oh something went wrong...");
			return 0;
		}
	}
	else {
		window = glfwCreateWindow(width, height, "Engine", NULL, NULL);
		if (!window)
		{
			camera.Zoom = 75;
			spdlog::error("Uh oh something went wrong...");
			return 0;
		}
	}
	

	// Set the current context to the openGL window
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Lock cursor to window
	if (camera.GetFreeLook())
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	

	// Load GL Libraries
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		spdlog::error("Failed to initialize GLAD");
		return -1;
	}

	glEnable(GL_STENCIL_TEST);

	// Display GLFW Version
	spdlog::info("GLFW Version : {}", glfwGetVersionString());

	// Display OpenGL Version
	const GLubyte* glVersion = glGetString(GL_VERSION);
	const GLubyte* glRenderer = glGetString(GL_RENDERER);
	
	spdlog::info("OpenGL Version: {}", reinterpret_cast<const char*>(glVersion));
	spdlog::info("Renderer: {}", reinterpret_cast<const char*>(glRenderer));

	Shader shaders("..\\Engine\\Shaders\\LitMaterial_Shader.vert", "..\\Engine\\Shaders\\LitMaterial_Shader.frag");
	Shader lightShader("..\\Engine\\Shaders\\lightSource.vert", "..\\Engine\\Shaders\\lightSource.frag");
	Shader stencilShader("..\\Engine\\Shaders\\LitMaterial_Shader.vert", "..\\Engine\\Shaders\\shaderSingleColor.frag");
	TempShader = shaders;
	lightCubeShader = lightShader;
	stencilShader = this->stencilShader;

	
	// Initialize IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	

	glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
	glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
	glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
	glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);

	// Load default light model to render
	
	Model LightSourceObj("../Engine/Models/Light_Cube.fbx");

	LightSourceObj.SetScale(glm::vec3(0.25f, 0.25f, 0.25f));
	//LightSourceObj.UpdateModelMatrix();
	LightSourceObj.IsLight = true;
	LightSourceObj.SetVisible(false);
	std::string dLight = "LightSource";
	LightSourceObj.SetModelName(dLight);
	ModelList.push_back(LightSourceObj);

	for (int i = 0; i < ModelList.size(); i++)
	{
		Model& modelitem = ModelList[i];
		modelitem.SetPosition(glm::vec3(-i + .5f, 0.0f, 0.0f));
		modelitem.UpdateModelMatrix();
	}

	if (ModelList.size() > 0)
		selectedDebugModelIndex = 0;


	//Initialize camera after window creation to update framebuffersize for imguizmo
	camera.Initialize(window);
	return 0;
}
int Backend::Update()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	int diW, diH;
	glfwGetFramebufferSize(window, &diW, &diH);
	FrameBuffer sceneBuf(width, height);

	// Main Loop *CORE*
	while (!glfwWindowShouldClose(window))
	{
		
		glfwPollEvents(); // Start Frame
		PollInputs(window);

		// Begin ImGui Inits
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		sceneBuf.Bind();
		//ImGui::DockSpaceOverViewport(ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::DockSpace(ImGui::GetID("Dockspace1"), ImVec2(0, 0),ImGuiDockNodeFlags_PassthruCentralNode);
		ImGuiWindowFlags hiddenWindowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_MenuBar;

		glm::mat4 identit(1.0f);
		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		
		camera.UpdateViewAndProjectionMatrices();
		//ImGui::SetNextWindowPos(ImVec2(0, 0));
		//ImGui::SetNextWindowSize(ImVec2(float(GetWindowWidth(window)), float(GetWindowHeight(window))));

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(float(GetWindowWidth(window)), float(GetWindowHeight(window))));
		ImGui::Begin("Engine", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar );
		ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0, 0));

		// Set the OpenGL viewport using the ImGui viewport's position and size
		


		ImGui::Begin("Scene");

		//ImGuizmo::DrawGrid(glm::value_ptr(view), glm::value_ptr(projection), glm::value_ptr(identit), 100.0f);
		

		//ImGui::BeginChild("GameRender");

		float winwidth = ImGui::GetContentRegionAvail().x;
		float winheight = ImGui::GetContentRegionAvail().y;

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, (GLsizei)winwidth, (GLsizei)winheight);
		sceneBuf.RescaleFrameBuffer(winwidth, winheight);
		
		ImGui::Image(
			(ImTextureID)sceneBuf.getFrameTexture(),
			ImGui::GetContentRegionAvail(),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);
		

		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetWindowSize();
		// 
		// End Imgui Inits

		float currentFrame = glfwGetTime();  // Get the current time
		deltaTime = currentFrame - lastFrame; // Calculate delta time
		lastFrame = currentFrame;  // Update last frame time

		if (shouldSpin) {
			if (!reverseSpin)
				rotationAngle += 0.5f;
			else
				rotationAngle -= 0.5f;

			if (rotationAngle > 360.0f) {
				rotationAngle -= 360.0f;
			}
			if (rotationAngle < 0.0f) {
				rotationAngle += 360.0f;
			}
		}

		
		ImGuizmo::BeginFrame();

		// Gizmo Manipulation
		if (DebugSelectedObj != nullptr)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
			
			glm::mat4* modelMatrix = &DebugSelectedObj->GetModelMatrix();
			ImGuizmo::Manipulate(glm::value_ptr(camera.GetViewMatrix()), glm::value_ptr(camera.GetProjectionMatrix()),
				myOperation,
				ImGuizmo::LOCAL,
				glm::value_ptr(*modelMatrix));
		}
		
		if (!ModelList.empty())
		{
			// lights first, models second
			for (int i = 0; i < ModelList.size(); i++) {
				Model& modelitem = ModelList[i];

				if (modelitem.IsLight) {
					DirectionalLightObject = &modelitem;

					// Shader setup for the light objects
					lightCubeShader.use();
					lightCubeShader.setMat4("model", modelitem.GetModelMatrix());
					lightCubeShader.setMat4("projection", camera.GetProjectionMatrix());
					lightCubeShader.setMat4("view", camera.GetViewMatrix());
					modelitem.SetShader(lightCubeShader);

					// Draw light objects *** Do we even need to draw lights?
					modelitem.Draw();
				}
				else
				{
					if (!modelitem.IsLight && modelitem.GetVisible()) {

						// Debug Spin model in editor
						if (shouldSpin)
						{
							modelitem.SetRotation(-rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
							modelitem.UpdateModelMatrix();
						}

						// Shader setup for lit models
						TempShader.use();
						TempShader.setMat4("model", modelitem.GetModelMatrix());
						TempShader.setMat4("projection", camera.GetProjectionMatrix());
						TempShader.setMat4("view", camera.GetViewMatrix());
						TempShader.setVec3("lightPos", DirectionalLightObject->GetPosition());
						TempShader.setVec3("viewPos", camera.Position);
						TempShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
						TempShader.setVec3("objectColor", 1.0f, .5f, .31f);
						modelitem.SetShader(TempShader);

						// Draw the model item
						modelitem.Draw();
					}
				}
			}

			// Models second
			for (int i = 0; i < ModelList.size(); i++) {
				Model& modelitem = ModelList[i];
				// pending optimization
				
			}
		}
		
		DebugModelList = &ModelList;

		sceneBuf.Unbind();
		//ImGui::EndChild();
		ImGui::End();

		DebugWindow(io);
		sceneBuf.RescaleFrameBuffer(winwidth, winheight);
		glfwSwapBuffers(window); // End Frame
	}
	

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
bool Backend::IsFullscreen = false;
Backend::Backend()
{
	spdlog::info("Initializing Backend");
}

 //																									Definitions

void Backend::DebugWindow(ImGuiIO& io)
{
	
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) 
			{ 
				Model newModel = OpenModelFileDialog(); 
				if (newModel.GetModelName() != "null_model")
				{
					ModelList.push_back(newModel);
				}
				
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
		
		//ImGui::AlignTextToFramePadding();
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
				LoggingWindowEntries.push_back(fmt::format("{} Selected", ModelList[i].GetModelName().c_str()));
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

				//ImGui::SetKeyboardFocusHere();
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
				LoggingWindowEntries.push_back("Keep in mind this duplicate is not instanced :(");
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
		ImGui::Text("X position: %.2f", camera.Position.x);
		ImGui::Text("Y position: %.2f", camera.Position.y);
		ImGui::Text("Z position: %.2f", camera.Position.z);

		// Camera Rotation
		ImGui::Text("Yaw: %.2f", camera.Yaw);
		ImGui::Text("Pitch: %.2f", camera.Pitch);

		// Field of view
		ImGui::Text("Field of View (FOV):");
		ImGui::SameLine();
		ImGui::SliderFloat("##fov", &camera.Zoom, 0, 100);

		// Near clipping
		ImGui::Text("Camera near clipping: ");
		ImGui::SameLine();
		ImGui::InputFloat("##nearClipping", &camera.NearClippingPlane);

		// Far clipping
		ImGui::Text("Camera far clipping: ");
		ImGui::SameLine();
		ImGui::InputFloat("##farClipping", &camera.FarClippingPlane);

		// Adjust camera Speed
		ImGui::Text("Camera Speed: ");
		ImGui::SameLine();
		ImGui::SliderFloat("Camera Speed", &camera.MovementSpeed, camera.Min_MoveSpeed, camera.Max_MoveSpeed);

		// Reverse Spin
		ImGui::Text("Reverse Spin? ");
		ImGui::SameLine();
		ImGui::Checkbox("##reverse", &reverseSpin);

		// Disable/enable spinning
		ImGui::Text("Spin? ");
		ImGui::SameLine();
		ImGui::Checkbox("##spin", &shouldSpin);
		

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
		for (int i = 0; i < LoggingWindowEntries.size(); i++)
		{
			ImGui::Text(LoggingWindowEntries[i].c_str());
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
void Show_UI()
{
	DEBUG_MODE = true;
}

// Quick hide function for debug ui elements
void Hide_UI()
{
	DEBUG_MODE = false;
}

// super useful for importing files from disk
Model Backend::OpenModelFileDialog()
{
																							// To Do: solve cancellation logic

	// Buffer to hold the file name
	wchar_t fileName[MAX_PATH] = L"";
	std::string logMsg;

	

	// Initialize OPENFILENAME structure
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;  // If you have a window handle, pass it here
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName) / sizeof(fileName[0]);  // Adjust for wide characters
	ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.TXT\0";
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
		LoggingWindowEntries.push_back(logMsg);

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
		LoggingWindowEntries.push_back(logMsg);
		Model model;
		return model;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

	glViewport(0, 0, width, height);
}

void Task_AlignDirLight()
{
	/*LoggingWindowEntries.push_back(fmt::format("Updated light position to \nx:{}\ny:{}\nz:{}", camera.Position.x * roundDecimal, camera.Position.y * roundDecimal, camera.Position.z * roundDecimal));*/
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2)
		<< "Updated light position to \nx:" << camera.Position.x
		<< "\ny:" << camera.Position.y
		<< "\nz:" << camera.Position.z;

	LoggingWindowEntries.push_back(oss.str());
	DirectionalLightObject->SetPosition(camera.Position);
	DirectionalLightObject->UpdateModelMatrix();
}

void Exit_Application(GLFWwindow* window)
{
	std::cout << "Goodbye!" << std::endl;

	glfwSetWindowShouldClose(window, true);
}

void ToggleFullscreen(GLFWwindow* window)
{
	if (Backend::IsFullscreen)
	{
		Backend::IsFullscreen = !Backend::IsFullscreen;
		spdlog::info(Backend::IsFullscreen);
		glfwSetWindowMonitor(window, NULL, 100, 100, Backend::width, Backend::height, 0);
		camera.Zoom = 45.0f;
	}
	else {
		Backend::IsFullscreen = !Backend::IsFullscreen;
		spdlog::info(Backend::IsFullscreen);
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 100, 100, Backend::full_width, Backend::full_height, 0);
		camera.Zoom = 90.0f;
	}
}

void LookAtObject(glm::vec3& ObjPosition)
{

	glm::vec3 zoffset = glm::vec3(0, 0, 3.0f);

	if (camera.Position != (ObjPosition - zoffset))
	{
		camera.Position = glm::vec3(ObjPosition.x, ObjPosition.y, ObjPosition.z - zoffset.z);
		camera.LookAtWithYaw(ObjPosition);
		camera.Pitch = -15.0f;
		camera.updateCameraVectors();
	}
}

void Task_FocusObject()
{
	if (DebugSelectedObj == nullptr)
	{
		LoggingWindowEntries.push_back("Debug Selected Object is empty (null)");
	}
	else
	{
		glm::vec3 ObjPosition = DebugSelectedObj->GetPosition();
		if (camera.Position != ObjPosition)
		{
			glm::vec3 ObjPosition = DebugSelectedObj->GetPosition();
			LookAtObject(ObjPosition);
			std::string message = std::format("Focused '{}' Object", DebugSelectedObj->GetModelName());
			LoggingWindowEntries.push_back(message);
		}
	}
}

void Task_DebugNormals(bool& flag, GLuint sId)
{
	flag = !flag;
	glUniform1i(glGetUniformLocation(sId, "DEBUG_NORMAL"), flag);
}

void Task_Delete()
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
		LoggingWindowEntries.push_back("No object selected!!");
	}
}

void PollInputs(GLFWwindow* window)
{

	if (camera.GetFreeLook())
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			camera.SetMovementSpeed(camera.BoostSpeed);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
			camera.SetMovementSpeed(camera.GetMovementSpeed());
	}
	glfwSetKeyCallback(window, Input_Callback);
	

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	static float lastX = 0, lastY = 0;
	static bool firstMouse = true;

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}


	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	if (camera.GetFreeLook())
		camera.ProcessMouseMovement(xoffset, yoffset);

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double x, y;

	glfwGetCursorPos(window, &x, &y);

	glm::vec4 viewport = glm::vec4(0, 0, Backend::width, Backend::height);
	glm::vec3 winPos = glm::vec3(x, Backend::height - y, 0.0f); // Near plane

	if ((button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) && !ImGui::IsAnyItemHovered())
	{
		
		
	}

	if ((button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) && !ImGui::IsAnyItemHovered())
	{
		camera.SetFreeLook(true);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (action == GLFW_RELEASE)
	{
		camera.SetFreeLook(false);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		if (ImGui::IsItemHovered())
		{
			ImGuiID myID = ImGui::GetHoveredID();
			std::string s = std::to_string(myID);
			LoggingWindowEntries.push_back(s);
		}
	}
}

void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_W:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			myOperation = ImGuizmo::OPERATION::TRANSLATE;
		}
		break;
	case GLFW_KEY_R:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			myOperation = ImGuizmo::OPERATION::ROTATE;
		}
		break;
	case GLFW_KEY_T:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			myOperation = ImGuizmo::OPERATION::SCALE;
		}
		break;
		// Enable/disable debug window
	case GLFW_KEY_H:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			DEBUG_MODE = !DEBUG_MODE;
		}
		break;
		// Hotkey to re-locate directional light
	case GLFW_KEY_G:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{

			Task_AlignDirLight();

		}

		break;
		// Stop the models from spinning 
	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			shouldSpin = !shouldSpin;
		}
		break;
		// Fullscreen hotkey
	case GLFW_KEY_F11:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			ToggleFullscreen(window);
		}
		break;
		// Stop the models from spinning 
	case GLFW_KEY_F:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			Task_FocusObject();
		}
		break;
	case GLFW_KEY_BACKSPACE:
		if (action == GLFW_PRESS && ImGui::IsAnyItemActive())
		{
			// remove last character from text boxes
		}
		break;
	case GLFW_KEY_ENTER:
		if (action == GLFW_PRESS && ImGui::IsAnyItemActive())
		{
			if (editingName)
			{
				editingNameLoggingMsg = fmt::format("{}", editingNameLoggingMsg);
				LoggingWindowEntries.push_back(fmt::format("{} to \"{}\" ", editingNameLoggingMsg, editingTempName));
				editingNameLoggingMsg = "";
				DebugSelectedObj->SetModelName(editingTempName);
				editingName = false;
			}
		}
		break;
	case GLFW_KEY_N:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			if (DebugSelectedObj != nullptr)
			{
				DebugSelectedObj->RenderMode = RENDERTARGETS::NORMAL;
				Task_DebugNormals(DEBUG_NORMAL_MAP, DebugSelectedObj->GetShaderID());
				LoggingWindowEntries.push_back(fmt::format("Show normals: {}", DEBUG_NORMAL_MAP));
			}
			else
			{
				LoggingWindowEntries.push_back("No object with shader selected!!");
			}
		}
		break;
	case GLFW_KEY_L:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			if (DebugSelectedObj != nullptr)
			{
				DebugSelectedObj->RenderMode = RENDERTARGETS::LINES;

				LoggingWindowEntries.push_back(fmt::format("Rendering lines"));
			}
			else
			{
				LoggingWindowEntries.push_back("No object selected!!");
			}
		}
		break;

		case GLFW_KEY_DELETE:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			Task_Delete();
		}
		break;
		// Quit out of program
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			Exit_Application(window);
		}
		break;
	}
}

