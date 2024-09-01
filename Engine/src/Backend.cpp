#include "../Headers/Backend.h"

void PollInputs(GLFWwindow* window);


//																							Backend Global variables 
bool DEBUG_MODE = true;
bool shouldSpin = true;
bool reverseSpin = false;
float deltaTime;
Camera camera;
glm::mat4 Backend::view = glm::mat4(1.0f);       
glm::mat4 Backend::projection = glm::mat4(1.0f);
Model* DirectionalLightObject;
Model* DebugSelectedObj;

// Session logging vector
std::vector<std::string> LoggingWindowEntries;

//																								 Function protos
std::string WideStringToString(const std::wstring& wstr);
void Task_AlignDirLight();
void LookAtObject(glm::vec3& ObjPosition);
void Task_FocusObject();
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
	projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
	view = camera.GetViewMatrix();
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

	// Display GLFW Version
	spdlog::info("GLFW Version : {}", glfwGetVersionString());

	// Display OpenGL Version
	const GLubyte* glVersion = glGetString(GL_VERSION);
	const GLubyte* glRenderer = glGetString(GL_RENDERER);
	
	spdlog::info("OpenGL Version: {}", reinterpret_cast<const char*>(glVersion));
	spdlog::info("Renderer: {}", reinterpret_cast<const char*>(glRenderer));

	Shader shaders("..\\Engine\\Shaders\\LitMaterial_Shader.vert", "..\\Engine\\Shaders\\LitMaterial_Shader.frag");
	Shader lightShader("..\\Engine\\Shaders\\lightSource.vert", "..\\Engine\\Shaders\\lightSource.frag");
	TempShader = shaders;
	lightCubeShader = lightShader;

	// Initialize IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	

	// Load models to render
	
	//Model ourModel1("../Engine/Models/Table.obj");
	//Model MonkeyMan("../Engine/Models/Monkey.obj");
	//Model Building("../Engine/Models/Building.obj");
	Model Backpack("../Engine/Models/backpack.obj");
	Model LightSourceObj("../Engine/Models/Light_Cube.fbx");
	/*ourModel1.SetScale(glm::vec3(.25f, .25f, .25f));
	ourModel1.UpdateModelMatrix();
	ModelList.push_back(ourModel1);

	MonkeyMan.SetScale(glm::vec3(.25f, .25f, .25f));
	MonkeyMan.UpdateModelMatrix();
	ModelList.push_back(MonkeyMan);

	Building.SetScale(glm::vec3(.25f, .25f, .25f));
	Building.UpdateModelMatrix();
	ModelList.push_back(Building);*/

	LightSourceObj.SetScale(glm::vec3(0.25f, 0.25f, 0.25f));
	LightSourceObj.UpdateModelMatrix();
	LightSourceObj.IsLight = true;
	LightSourceObj.SetVisible(false);
	ModelList.push_back(LightSourceObj);

	Backpack.SetScale(glm::vec3(.25f, .25f, .25f));
	Backpack.UpdateModelMatrix();
	ModelList.push_back(Backpack);

	for (int i = 0; i < ModelList.size(); i++)
	{
		Model& modelitem = ModelList[i];
		modelitem.SetPosition(glm::vec3(-i + .5f, 0.0f, 0.0f));
		modelitem.UpdateModelMatrix();
	}
	//ModelList.push_back(Building);

	return 0;
}
int Backend::Update()
{
	
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	

	// Main Loop *CORE*
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); // Start Frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		PollInputs(window);

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

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

		projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		

		// lights first
		for (int i = 0; i < ModelList.size(); i++) {
			Model& modelitem = ModelList[i];

			if (modelitem.IsLight) {
				DirectionalLightObject = &modelitem;

				lightCubeShader.use();
				lightCubeShader.setMat4("model", modelitem.GetModelMatrix());
				lightCubeShader.setMat4("projection", projection);
				lightCubeShader.setMat4("view", view);

				modelitem.Draw(lightCubeShader);
			}
		}

		// Models second
		for (int i = 0; i < ModelList.size(); i++) {
			Model& modelitem = ModelList[i];

			if (!modelitem.IsLight) {

				if (shouldSpin)
				{
					modelitem.SetRotation(-rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
					modelitem.UpdateModelMatrix();
				}
				TempShader.use();
				TempShader.setMat4("model", modelitem.GetModelMatrix());
				TempShader.setMat4("projection", projection);
				TempShader.setMat4("view", view);
				TempShader.setVec3("lightPos", DirectionalLightObject->GetPosition());
				TempShader.setVec3("viewPos", camera.Position);
				TempShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
				TempShader.setVec3("objectColor", 1.0f, .5f, .31f);

				modelitem.Draw(TempShader);
			}
		}

		DebugWindow(io);

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
	static float scaleX = 1.0f;
	static float scaleY = 1.0f;
	static float scaleZ = 1.0f;

	
	
	ImGuiWindowFlags hiddenWindowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus |                 
		ImGuiWindowFlags_NoNavFocus |                                                      
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoBackground;

	
	ImGui::SetNextWindowPos(ImVec2(0, 0));                                                  
	ImGui::SetNextWindowSize(ImVec2(float(GetWindowWidth(window)), float(GetWindowHeight(window))));
	
	ImGui::Begin("Engine", 0, hiddenWindowFlags);
	ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) 
			{ 
				Model newModel = OpenModelFileDialog(); 
				ModelList.push_back(newModel);
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) { spdlog::info("ToDo: Save scene? Maybe, no solid plans yet."); }
			if (ImGui::MenuItem("Close", "'Esc'")) { Exit_Application(window); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Set Light Position to Camera", "'G'")) { Task_AlignDirLight(); }
			if (ImGui::MenuItem("Focus Camera to Selected", "'F'")) { Task_FocusObject(); }
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

	{
		
		ImGui::Begin("Properties");
		ImGuiInputTextFlags textFlags = ImGuiInputTextFlags_EnterReturnsTrue;
		for (int i = 0; i < ModelList.size(); i++)
		{
			Model& model = ModelList[i];
			glm::vec3& modelPosition = model.GetPosition();
			glm::vec3& modelScale = model.GetScale();
			glm::vec3& modelRotation = model.GetRotation();
			if (i == selectedDebugModelIndex)
			{
				ImGui::Text("Model Name: ");
				char test[12];
				strcpy_s(test, model.GetModelName().c_str());
				if (ImGui::InputText(" ", test, IM_ARRAYSIZE(test)))
				{
					DebugSelectedObj->SetModelName(std::string(test));
				}

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
		
		ImGui::AlignTextToFramePadding();
		ImGui::End();
	}

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
		}
		ImGui::AlignTextToFramePadding();
		ImGui::End();
	}

	{
		ImGui::Begin("Debug");
		if (ImGui::Button("Toggle Fullscreen Mode"))
			ToggleFullscreen(window);


		ImGui::Text("Camera:");
		ImGui::Text("X position: %.2f", camera.Position.x);
		ImGui::Text("Y position: %.2f", camera.Position.y);
		ImGui::Text("Z position: %.2f", camera.Position.z);

		ImGui::Text("Yaw: %.2f", camera.Yaw);
		ImGui::Text("Pitch: %.2f", camera.Pitch);
		ImGui::SliderFloat("Camera FOV", &camera.Zoom, 0, 100);

		ImGui::Checkbox("Reverse Direction", &reverseSpin);
		ImGui::Checkbox("Spin Model", &shouldSpin);
		//ImGuizmo::SetDrawlist();
		//ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, (float*)glm::value_ptr(ModelList[0].GetModelMatrix()));
		ImGui::SliderFloat("Camera Speed", &camera.MovementSpeed, camera.Min_MoveSpeed, camera.Max_MoveSpeed);
		//ImGui::ColorEdit1("Viewport Color", (float*)&clear_color);
		ImGui::ColorEdit3("Viewport Color", (float*)&clear_color);

		const GLubyte* glVersion = glGetString(GL_VERSION);
		const GLubyte* glRenderer = glGetString(GL_RENDERER);
		ImGui::Text("GPU: %s", glRenderer);
		ImGui::Text("Application %.1f FPS", io.Framerate);
		ImGui::Text("GLFW Version: %s", glfwGetVersionString());
		ImGui::Text("OpenGL Version: %s", glVersion);
		ImGui::End();
	}

	{
		ImGui::Begin("Logging");
		ImGui::Text("Logging window successfully initialized!");
		for (int i = 0; i < LoggingWindowEntries.size(); i++)
		{
			ImGui::Text(LoggingWindowEntries[i].c_str());
		}
		ImGui::End();
	}

	{
		ImGui::Begin("File Viewer");
		
		
		ImGui::End();
	}
	
	ImGui::Render();

	if (DEBUG_MODE)
	{
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

void Show_UI()
{
	DEBUG_MODE = true;
}

void Hide_UI()
{
	DEBUG_MODE = false;
}

std::string WideStringToString(const std::wstring& wstr)
{
	// Convert wstring to string
	std::string str(wstr.begin(), wstr.end());
	return str;
}

Model Backend::OpenModelFileDialog()
{
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
		std::string ofnName = WideStringToString(ofn.lpstrFile);
		std::filesystem::path path(ofnName);
		std::string extName = path.filename().string();
		

		// extTemp example value: ".obj", ".fbx"
		std::string extTemp = extName.substr(extName.length() - 4);

		// extName example value: "Backpack", "Cube"
		extName = extName.substr(0, extName.length() - 4);

		// Search if name exists
		for (int i = 0; i < ModelList.size(); i++)
		{
			// If it does, increment name
			if (ModelList[i].GetModelName() == extName)
			{
				extName = fmt::format("{}({})", extName, (i + 1));
			}
		}

		logMsg = fmt::format("Successfully loaded file: {}{}", extName, extTemp);
		spdlog::info(logMsg);
		LoggingWindowEntries.push_back(logMsg);

		// Load Model
		Model loadedModel(ofnName);
		loadedModel.SetModelName(extName);
		return loadedModel;
	}
	else
	{
		logMsg = "Open file selection cancelled";
		spdlog::info(logMsg);
		LoggingWindowEntries.push_back(logMsg);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Task_AlignDirLight()
{
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
	glm::vec3 nearPoint = glm::unProject(winPos, Backend::view, Backend::projection, viewport);
	winPos.z = 1.0f; // Far plane
	glm::vec3 farPoint = glm::unProject(winPos, Backend::view, Backend::projection, viewport);



	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// object picker to do
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		camera.SetFreeLook(true);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (action == GLFW_RELEASE)
	{
		camera.SetFreeLook(false);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (camera.GetFreeLook())
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	switch (key)
	{
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
		// Quit out of program
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			Exit_Application(window);
		}
		break;
	}
}

