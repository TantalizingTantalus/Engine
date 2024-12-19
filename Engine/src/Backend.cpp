#include "../Headers/Backend.h"


//									 Global variables 



Editor EditorWindow;
Camera camera;
Time EditorTime;

// testing
GLuint pickingFramebuffer;
GLuint pickingTexture;

void SetupPickingFramebuffer(int x, int y) {
	// Create the texture for storing entity IDs
	glGenTextures(1, &pickingTexture);
	glBindTexture(GL_TEXTURE_2D, pickingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, x, y, 0, GL_RED_INTEGER, GL_INT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create the framebuffer for picking
	glGenFramebuffers(1, &pickingFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Picking framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind framebuffer
}

struct Character {
	unsigned int TextureID; 
	glm::ivec2   Size;      
	glm::ivec2   Bearing;   
	unsigned int Advance;   
};

std::map<GLchar, Character> Characters;
unsigned int VAO, VBO;

//									 Function protos

void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color, float, float);
void InitializeUserInterface();

void PollInputs(GLFWwindow* window);
Backend::Backend()
{
	spdlog::info("Initializing Backend");
}


//																									Main Logic
int Backend::Initialize()
{
	EditorTime.deltaTime = 0.0f;
	EditorWindow.IsFullscreen = false;
	
	camera.Position = (glm::vec3(0.0f, 0.0f, 3.0f));
	spdlog::info("Initializing GLFW");

	// Window setup
	{
		// Initialize GLFW
		if (!glfwInit())
		{
			spdlog::error("GLFW failed intialization...");
			return 0;
		}

		if (EditorWindow.IsFullscreen)
		{
			m_Window = glfwCreateWindow(m_FullWidth, m_FullHeight, "Engine", glfwGetPrimaryMonitor(), NULL);
			if (!m_Window)
			{
				camera.Zoom = 95;
				spdlog::error("Uh oh something went wrong...");
				return 0;
			}
		}
		else {
			m_Window = glfwCreateWindow(m_Width, m_Height, "Engine", NULL, NULL);
			if (!m_Window)
			{
				camera.Zoom = 75;
				spdlog::error("Uh oh something went wrong...");
				return 0;
			}
		}
	}
	
	// Additional window setup
	{
		// Set the current context to the openGL window
		glfwMakeContextCurrent(m_Window);
		glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

		//Lock cursor to window
		if (camera.GetFreeLook())
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		glfwSetCursorPosCallback(m_Window, mouse_callback);
		glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
		glfwSetScrollCallback(m_Window, scroll_callback);

		// Set full height to glfw window
		GLFWmonitor* primMonitor = glfwGetPrimaryMonitor();
		if (primMonitor)
		{
			const GLFWvidmode* videoMode = glfwGetVideoMode(primMonitor);
			if (videoMode)
			{
				m_FullHeight = videoMode->height;
				m_FullWidth = videoMode->width;
			}
		}
	}

	// Load GL Libraries
	{
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			spdlog::error("Failed to initialize GLAD");
			return -1;
		}
		glEnable(GL_STENCIL_TEST);
	}

	// Debug display framework versioning
	{
		// Display GLFW Version
		spdlog::info("GLFW Version : {}", glfwGetVersionString());	
		// Display OpenGL Version
		const GLubyte* glVersion = glGetString(GL_VERSION);
		const GLubyte* glRenderer = glGetString(GL_RENDERER);
		spdlog::info("OpenGL Version: {}", reinterpret_cast<const char*>(glVersion));
		spdlog::info("Renderer: {}", reinterpret_cast<const char*>(glRenderer));
	}

	// Shader Setup
	{
		Shader shaders("../Engine/Shaders/LitMaterial_Shader.vert", "../Engine/Shaders/LitMaterial_Shader.frag");
		Shader lightShader("../Engine/Shaders/lightSource.vert", "../Engine/Shaders/lightSource.frag");
		//Shader stencilShader("../Engine/Shaders/LitMaterial_Shader.vert", "../Engine/Shaders/shaderSingleColor.frag");
		Shader myTextShader("../Engine/Shaders/TextGlyph.vert", "../Engine/Shaders/TextGlyph.frag");
		Shader shadShader("../Engine/Shaders/ShadowMapShader.vert", "../Engine/Shaders/EmptyFragment.frag");
		m_LitMaterialShader = shaders;
		m_LightShader = lightShader;
		m_TextShader = myTextShader;
		m_ShadowShader = shadShader;
	}
	
	// Initialize IMGUI
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}
	
	// Setup glfw input Callbacks
	{
		glfwSetCharCallback(m_Window, ImGui_ImplGlfw_CharCallback);
		glfwSetKeyCallback(m_Window, ImGui_ImplGlfw_KeyCallback);
		glfwSetMouseButtonCallback(m_Window, ImGui_ImplGlfw_MouseButtonCallback);
		glfwSetScrollCallback(m_Window, ImGui_ImplGlfw_ScrollCallback);
	}


	{
		// Create default light model
		std::string dLight = "LightSource";
		Model LightSourceModelComp("../Engine/Models/Light_Cube.fbx");
		Entity LightSourceEnt(dLight.c_str());
		LightSourceEnt.transform->setLocalScale(glm::vec3(0.25f, 0.25f, 0.25f));
		LightSourceModelComp.IsLight = true;

		// Add components to model
		LightSourceEnt.AddComponent(LightSourceEnt.transform);
		LightSourceEnt.GetComponent<Transform>().setLocalPosition(glm::vec3(2.8f, 1.0f, -2.5));
		std::shared_ptr<Model> LightSourceModel = std::make_shared<Model>(LightSourceModelComp);
		LightSourceEnt.AddComponent(LightSourceModel);
		LightSourceEnt.GetComponent<Model>().SetVisible(false);
		LightSourceEnt.GetComponent<Model>().parentEntity = &LightSourceEnt;
		std::shared_ptr<Light> light_Component = std::make_shared<Light>();
		LightSourceEnt.AddComponent(light_Component);
		LightSourceEnt.GetComponent<Light>().LightColor = ImVec4(0.0f / 255.0f, 4.0f / 255.0f, 251.0f / 255.0f, 1.0f);

		// Ship it
		LightSourceEnt.ID = ModelList.size() + 1;
		ModelList.push_back(LightSourceEnt);
		m_PointLights.push_back(LightSourceEnt);

		// Create default light model
		std::string dLight2 = "LightSource2";
		Model LightSourceModelComp2("../Engine/Models/Light_Cube.fbx");
		Entity LightSourceEnt2(dLight2.c_str());
		LightSourceEnt2.transform->setLocalScale(glm::vec3(0.25f, 0.25f, 0.25f));
		LightSourceModelComp2.IsLight = true;

		// Add components to model
		
		LightSourceEnt2.AddComponent(LightSourceEnt2.transform);
		LightSourceEnt2.GetComponent<Transform>().setLocalPosition(glm::vec3(-5.3f, 1.0f, 3.5f));
		std::shared_ptr<Model> LightSourceModel2 = std::make_shared<Model>(LightSourceModelComp2);
		LightSourceEnt2.AddComponent(LightSourceModel2);
		LightSourceEnt2.GetComponent<Model>().SetVisible(false);
		LightSourceEnt2.GetComponent<Model>().parentEntity = &LightSourceEnt2;
		std::shared_ptr<Light> light_Component2 = std::make_shared<Light>();
		LightSourceEnt2.AddComponent(light_Component2);
		LightSourceEnt2.GetComponent<Light>().LightColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

		// Ship it
		LightSourceEnt2.ID = ModelList.size() + 1;
		ModelList.push_back(LightSourceEnt2);
		m_PointLights.push_back(LightSourceEnt2);

		// Create default room model
		Model Room("../Engine/Models/Paveway.obj");
		Entity RoomEnt(Room.GetModelName().c_str());
		
		Material TheRoomMat(&RoomEnt);

		// Add components to model
		std::shared_ptr<Model> RoomModelComp = std::make_shared<Model>(Room);
		std::shared_ptr<Material> RoomMat = std::make_shared<Material>(TheRoomMat);
		RoomEnt.AddComponent(RoomEnt.transform);
		RoomEnt.AddComponent(RoomModelComp);
		RoomEnt.AddComponent(RoomMat);
		RoomEnt.GetComponent<Model>().parentEntity = &RoomEnt;
		RoomEnt.GetComponent<Material>().Initialize(RoomEnt.GetComponent<Model>());
		RoomEnt.GetComponent<Material>().pModel->SetShader(m_LitMaterialShader);
		RoomEnt.ID = ModelList.size() + 1;

		// Ship it
		ModelList.push_back(RoomEnt);
	}

	// Position default render list (mostly unused unless for a demo)
	{
		/*for (int i = 0; i < ModelList.size(); i++)
		{
			Entity& modelitem = ModelList[i];
			modelitem.transform->setLocalPosition(glm::vec3(-i + .5f, 0.0f, 0.0f));
		}*/
	}

	// Auto select the first item in the render list for manipulation.
	if (ModelList.size() > 0)
		EditorWindow.DebugSelectedEntity = &ModelList[0];

	//Initialize camera after window creation to update framebuffersize for imguizmo
	camera.Initialize(m_Window);
	
	camera.Position = glm::vec3(.6f, .83f, 1.3f);

	// return 1 for complete inits
	return 1;
}

int Backend::Update()
{
	// This loads the window layout from default file
	std::ifstream src("../Engine/DefaultLayout.ini", std::ios::binary);
	std::ofstream dst("../Engine/imgui.ini", std::ios::binary);

	std::filesystem::path defaultLayoutPath = "../Engine/DefaultLayout.ini";
	std::filesystem::path imGuiLayoutPath = "../Engine/imgui.ini";

	if (!std::filesystem::exists(defaultLayoutPath))
	{
		spdlog::error("Default ini could not be loaded");
		
	}
	
	try
	{
		std::filesystem::copy_file(defaultLayoutPath, imGuiLayoutPath, std::filesystem::copy_options::overwrite_existing);

	}
	catch (const std::filesystem::filesystem_error& err)
	{
		spdlog::error(fmt::format("Ran into issues while loading defaultlayout.ini, error here:\n{}", err.what()));
	}
	// Reload the layout settings from imgui.ini
	ImGui::LoadIniSettingsFromDisk("imgui.ini");

	ImGui::GetCurrentContext()->Windows.clear();
	ImGui::MarkIniSettingsDirty();

	// Last minute ImGui io setup
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), m_EditorSetting_FontSize);
	ImGuiStyle& Style = ImGui::GetStyle();
	Style.WindowMenuButtonPosition = ImGuiDir_None;

	// Setup UI
	InitializeUserInterface();

	// The lil fluffy icon mascot
	LoadEngineIcon();

	// Create Framebuffer
	FrameBuffer sceneBuf(m_Width, m_Height);


	// Picking texture creation
	GLuint pickingTexture;
	glGenTextures(1, &pickingTexture);
	glBindTexture(GL_TEXTURE_2D, pickingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Attach picking texture to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, sceneBuf.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, pickingTexture, 0);

	// Specify the draw buffers
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Stencil buffer creation
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// Stencil shader setup - todo
	Shader stencilShader("../Engine/Shaders/shaderSingleColor.vert", "../Engine/Shaders/shaderSingleColor.frag");
	m_StencilShader = stencilShader;

	// Bind framebuffer for config
	glBindFramebuffer(GL_FRAMEBUFFER, sceneBuf.fbo);

	// Attach color textures as in your code
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, pickingTexture, 0);

	// Create and attach the stencil buffer
	GLuint stencilRenderbufferID;
	glGenRenderbuffers(1, &stencilRenderbufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, stencilRenderbufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height); // or GL_STENCIL_INDEX8 for stencil only
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRenderbufferID);

	// Shadow map buffer creation
	//unsigned int depthMapFBO;
	//glGenFramebuffers(1, &depthMapFBO);

	//const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	//unsigned int depthMap;
	//glGenTextures(1, &depthMap);
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
	//	SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//glDrawBuffer(GL_NONE); // No color output in the bound framebuffer
	//glReadBuffer(GL_NONE);

	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Framebuffer not complete!" << std::endl;
	}

	// Unbind Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Scene vars
	int prevSceneWidth = 0, prevSceneHeight = 0;

	// Initialize the imgui editor windows
	EditorWindow.Init(GetBackEnd());


	// Pre-Load directional light
	m_LitMaterialShader.use();
	m_LitMaterialShader.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
	m_LitMaterialShader.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
	m_LitMaterialShader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	m_LitMaterialShader.setFloat("dirLight.intensity", MyDirLight.m_Intensity);
	m_LitMaterialShader.setBool("dirLight.inUse", MyDirLight.isActive);
	MyDirLight.m_DirShader = &m_LitMaterialShader;
	for (int i = 0; i < m_PointLights.size(); i++) {

		// pre-load point lights
		m_LitMaterialShader.use();
		m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].ambient", i), 0.05f, 0.05f, 0.05f);
		m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].diffuse", i), 0.8f, 0.8f, 0.8f);
		m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].specular", i), 1.0f, 1.0f, 1.0f);
		m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].intensity", i), m_PointLights[i].GetComponent<Light>().lightIntensity);
		m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].constant", i), 1.0f);
		m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].linear", i), 0.09f);
		m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].quadratic", i), 0.032f);
	}

	// Initialize icons used by the renderer/editor
	SystemIcons::Initialize();

	float near_plane = 1.0f, far_plane = 7.5f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	// Main Loop *CORE*
	while (!glfwWindowShouldClose(m_Window))
	{
		// Start Frame
		glfwPollEvents();
		PollInputs(m_Window);

		m_LitMaterialShader.use();
		m_LitMaterialShader.setVec3("dirLight.direction", MyDirLight.m_Direction);
		m_LitMaterialShader.setVec3("dirLight.color", glm::vec3(MyDirLight.m_Color.x, MyDirLight.m_Color.y, MyDirLight.m_Color.z));
		m_LitMaterialShader.setFloat("dirLight.intensity", MyDirLight.m_Intensity);

		// Point light update, quite expensive at the moment
		for (int i = 0; i < m_PointLights.size(); i++)
		{
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].intensity", i), m_PointLights[i].GetComponent<Light>().lightIntensity);
			m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].color", i), glm::vec3(m_PointLights[i].GetComponent<Light>().LightColor.x, m_PointLights[i].GetComponent<Light>().LightColor.y, m_PointLights[i].GetComponent<Light>().LightColor.z));
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].constant", i), 1.0f);
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].linear", i), 0.09f);
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].quadratic", i), 0.032f);

		}

		// Begin ImGui Inits
		StartImGui();

		// Bind framebuffer
		sceneBuf.Bind();

		// Docking space for ImGui setup
		UpdateDockingScene();

		// Begin Scene window frame
		ImGui::Begin("Scene", nullptr);

		ImVec2 scenePos = ImGui::GetWindowPos();
		ImVec2 sceneSize = ImGui::GetWindowSize();

		// Get the mouse position from ImGui's IO
		ImVec2 mousePos = ImGui::GetIO().MousePos;

		

		// Check if the mouse is inside the Scene Window's boundaries
		if (mousePos.x >= scenePos.x && mousePos.x <= scenePos.x + sceneSize.x &&
			mousePos.y >= scenePos.y && mousePos.y <= scenePos.y + sceneSize.y) {
			EditorWindow.m_SceneHovered = true;
		}
		else {
			EditorWindow.m_SceneHovered = false;
		}

		// Size scene to ImGui window
		m_SceneWidth = ImGui::GetContentRegionAvail().x;
		m_SceneHeight = ImGui::GetContentRegionAvail().y;

		// If screen is resized, update the picking texture
		if (m_SceneWidth != prevSceneWidth || m_SceneHeight != prevSceneHeight) {
			glBindTexture(GL_TEXTURE_2D, pickingTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_SceneWidth, m_SceneHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);

			sceneBuf.RescaleFrameBuffer(m_SceneWidth, m_SceneHeight);
			prevSceneWidth = m_SceneWidth;
			prevSceneHeight = m_SceneHeight;
		}

		glViewport(0, 0, (GLsizei)m_SceneWidth, (GLsizei)m_SceneHeight);

		// To do: Implement Depth Map (Shadow maps)
		// 
		// 1. first render to depth map
		//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		//glClear(GL_DEPTH_BUFFER_BIT);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//// 2. then render scene as normal with shadow mapping 
		//glViewport(0, 0, m_SceneWidth, m_SceneHeight);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glBindTexture(GL_TEXTURE_2D, depthMap);

		ImGui::Image(
			(ImTextureID)sceneBuf.getFrameTexture(),
			ImGui::GetContentRegionAvail(),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);

		// Time tracking
		EditorTime.Update();

		// Editor window setup
		EditorWindow.WindowUpdate(camera, *m_Window);

		HandleMouseClick(sceneBuf.fbo);

		// Render ModelList
		RenderModels();

		// Begin Game UI
		RenderUI();

		// End Render scene
		ImGui::End();

		// Unbind the scene buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		sceneBuf.Unbind();

		// Send io for editor menus
		EditorWindow.DebugWindow(io, ModelList);

		// Final
		glfwSwapBuffers(m_Window);
	}

	// Cleanup
	SystemIcons::Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	// Return out
	return 1;
}

 //												Definitions

bool Backend::UpdateDockingScene()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(float(GetWindowWidth(m_Window)), float(GetWindowHeight(m_Window))));
	ImGui::Begin("Engine", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0, 0));
	ImGui::DockSpace(ImGui::GetID("Dockspace1"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

	return true;
}

bool Backend::LoadEngineIcon()
{
	// Load the image
	int width, height, channels;
	unsigned char* data = stbi_load("../Engine/icon.png", &width, &height, &channels, 4);
	if (!data) {
		spdlog::error("ran into issues loading window icon...");
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		return -1;
	}

	// Create the glfwImage
	GLFWimage icon;
	icon.width = width;
	icon.height = height;
	icon.pixels = data;

	// Set the window icon
	glfwSetWindowIcon(m_Window, 1, &icon);

	stbi_image_free(data);
	return true;
}

bool Backend::StartImGui()
{
	try
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		return true;
	}
	catch (std::exception Ex)
	{
		spdlog::error("Ran into issues starting the ImGui frame...");
		return false;
	}
}

bool Backend::RenderUI()
{
	ImVec2 windowSize = ImGui::GetWindowSize();

	// Money increment logic
	if (EditorTime.currentTime >= 5.0f) {
		money += 15;
		EditorTime.currentTime -= 5.0f;
	}

	if (EditorWindow.renderUI)
	{
		RenderText(m_TextShader, fmt::format("Time: {:.2f}", EditorTime.currentFrame), (windowSize.x / windowSize.x) + 25.0f, windowSize.y - 40.0f, .85f, glm::vec3(0.5, 0.8f, 0.2f), windowSize.x, windowSize.y);
		RenderText(m_TextShader, fmt::format("Money: {:.2f}", money), windowSize.x - (windowSize.x / 3) - 25.0f, windowSize.y - 40.0f, .85f, glm::vec3(0.5, 0.8f, 0.2f), windowSize.x, windowSize.y);
	}

	
	return true;
}

void Backend::SelectEntity(int id)
{
	for (auto& p : *EditorWindow.DebugEntityList)
	{
		if (p.ID == id)
		{
			EditorWindow.DebugSelectedEntity = &p;
			if (EditorWindow.camera)
			{
				EditorWindow.camera->OrbitTarget = p.GetComponent<Transform>().position;
			}
		}
			
	}
}

void Backend::HandleMouseClick(GLuint framebuffer) {
	auto& io = ImGui::GetIO();
	
	if (ImGui::IsMouseClicked(0) && !ImGuizmo::IsOver()) {
		ImVec2 mousePos = ImGui::GetMousePos();

		int mouseX = static_cast<int>(mousePos.x - ImGui::GetWindowPos().x);
		int mouseY = static_cast<int>(mousePos.y - ImGui::GetWindowPos().y);

		mouseY = m_SceneHeight - mouseY;

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glReadBuffer(GL_COLOR_ATTACHMENT1);

		GLubyte pixel[4] = { 0 };
		glReadPixels(mouseX, mouseY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		
		int entityID = pixel[0] + pixel[1] * 256 + pixel[2] * 256 * 256;


		if (entityID > 0) {
			SelectEntity(entityID);
		}
	}
}

bool Backend::RenderModels()
{
	try
	{
		// Update camera matrices every frame
		camera.UpdateViewAndProjectionMatrices();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glClearColor(EditorWindow.clear_color.x, EditorWindow.clear_color.y, EditorWindow.clear_color.z, EditorWindow.clear_color.w);

		// Begin Gizmo frame
		ImGuizmo::BeginFrame();

		// Gizmo Manipulation
		if (EditorWindow.DebugSelectedEntity != nullptr)
		{
			

			if (camera.mode == Camera_Mode::ORTHO)
			{
				ImGuizmo::SetOrthographic(true);
				
			}
			else
			{
				ImGuizmo::SetOrthographic(false);
			}
				
			ImGuizmo::SetDrawlist();

			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

			glm::mat4& modelMatrix = EditorWindow.DebugSelectedEntity->transform->m_modelMatrix;

			if (ImGuizmo::Manipulate( 
				glm::value_ptr(camera.GetViewMatrix()),
				glm::value_ptr(camera.GetProjectionMatrix()),
				EditorWindow.myOperation,
				ImGuizmo::WORLD,
				glm::value_ptr(modelMatrix)))
			{
				glm::vec3 translation = glm::vec3(1.0f), rotation = glm::vec3(1.0f), scale = glm::vec3(1.0f);
				glm::vec3 deltaRotation = rotation - EditorWindow.DebugSelectedEntity->transform->getLocalRotation();
			
				if (ImGuizmo::IsUsing())
				{
					EditorWindow.DebugSelectedEntity->transform->DecomposeTransform(modelMatrix, translation, rotation, scale);
					EditorWindow.DebugSelectedEntity->transform->position = translation;
					EditorWindow.DebugSelectedEntity->transform->rotation += deltaRotation;
					EditorWindow.DebugSelectedEntity->transform->scale = scale;
					
				}	
			}
		}

		for (auto& light : ModelList)
		{
			if (light.HasComponent<Light>())
			{
				if (light.GetComponent<Light>().lightType == LightType::POINTLIGHT)
				{
					if (std::find(m_PointLights.begin(), m_PointLights.end(), light) == m_PointLights.end())
					{
						m_PointLights.push_back(light);
					}

				}

			}
		}

		// Render Models
		if (!ModelList.empty())
		{
			for (int i = 0; i < m_PointLights.size(); i++)
			{
				m_LitMaterialShader.use();
				m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].position", i), m_PointLights[i].GetComponent<Transform>().position);
				m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].color", i), glm::vec3(m_PointLights[i].GetComponent<Light>().LightColor.x, m_PointLights[i].GetComponent<Light>().LightColor.y, m_PointLights[i].GetComponent<Light>().LightColor.z));
			}
			
			
			ImVec4* DirectionalColor = nullptr;
			// Render lights first, models second
			for (int i = 0; i < ModelList.size(); i++) {
				Entity& modelItem = ModelList[i];
				
				modelItem.GetComponent<Model>().SetShader(m_LightShader);
				if (modelItem.HasComponent<Light>()) {
					
					// Shader setup for the light objects
					m_LightShader.use();
					m_LightShader.setMat4("model", modelItem.GetComponent<Transform>().m_modelMatrix);
					m_LightShader.setMat4("projection", camera.GetProjectionMatrix());
					m_LightShader.setMat4("view", camera.GetViewMatrix());
					m_LightShader.setInt("entityID", modelItem.ID);
					DirectionalColor = &modelItem.GetComponent<Light>().LightColor;

					modelItem.GetComponent<Model>().SetShader(m_LightShader);

					// Draw light objects
					modelItem.GetComponent<Model>().Draw();
				}
				else
				{
					// Render Models second
					if (!modelItem.HasComponent<Light>() && modelItem.GetComponent<Model>().GetVisible()) {
						glEnable(GL_STENCIL_TEST);
						glStencilFunc(GL_ALWAYS, 1, 0xFF); 
						glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 
						glStencilMask(0xFF); 
						glDepthMask(GL_TRUE); 
						glEnable(GL_DEPTH_TEST);
						modelItem.GetComponent<Model>().SetShader(m_LitMaterialShader);
						// Shader setup for lit models
						m_LitMaterialShader.use();
						
						m_LitMaterialShader.setMat4("model", modelItem.transform->m_modelMatrix);
						m_LitMaterialShader.setMat4("projection", camera.GetProjectionMatrix());
						m_LitMaterialShader.setMat4("view", camera.GetViewMatrix());
						m_LitMaterialShader.setVec3("viewPos", camera.Position);
						if (modelItem.HasComponent<Material>())
						{
							m_LitMaterialShader.setBool("material.hasSpecular", modelItem.GetComponent<Model>().hasSpecular);
							m_LitMaterialShader.setBool("material.hasNormal", modelItem.GetComponent<Model>().hasNormal);
							m_LitMaterialShader.setFloat("material.specularIntensity", modelItem.GetComponent<Material>().m_SpecIntensity);
							m_LitMaterialShader.setFloat("material.shininess", modelItem.GetComponent<Material>().m_Shininess);
						}
						m_LitMaterialShader.setInt("entityID", modelItem.ID);
						/*m_ShadowShader.use();
						m_ShadowShader.setMat4("model", modelItem.GetComponent<Transform>().m_modelMatrix);*/
						
						// Draw the model item, this is 1 draw call per frame 60fps = 60 draw calls
						modelItem.GetComponent<Model>().Draw();

						
						if (modelItem.ID == EditorWindow.DebugSelectedEntity->ID && !EditorWindow.DEBUG_NORMAL_MAP)
						{
							
							glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
							glStencilMask(0x00); 

							
							glDisable(GL_DEPTH_TEST);

							
							m_StencilShader.use();
							glm::mat4 scaledModelMatrix = glm::scale(
								modelItem.transform->m_modelMatrix,
								glm::vec3(EditorWindow.OutlineThickness, EditorWindow.OutlineThickness, EditorWindow.OutlineThickness)
							);
							m_StencilShader.setMat4("model", scaledModelMatrix);
							m_StencilShader.setMat4("projection", camera.GetProjectionMatrix());
							m_StencilShader.setMat4("view", camera.GetViewMatrix());
							m_StencilShader.setVec3("viewPos", camera.Position);
							m_StencilShader.setInt("entityID", modelItem.ID);
							/*m_ShadowShader.use();
							m_ShadowShader.setMat4("model", modelItem.GetComponent<Transform>().m_modelMatrix);*/
							
							modelItem.GetComponent<Model>().SetShader(m_StencilShader);
							modelItem.GetComponent<Model>().Draw();

							
							glEnable(GL_DEPTH_TEST);
							glStencilMask(0xFF); 
							glStencilFunc(GL_ALWAYS, 0, 0xFF); 
						}

					}

					glDisable(GL_STENCIL_TEST);
				}
			}
		}

		// Set debug modellist to current frame's model list
		EditorWindow.DebugEntityList = &ModelList;

		return true;
	}
	catch (std::exception ex)
	{
		spdlog::error("Ran into issues during RenderModels():\n{}", ex.what());
		return false;
	}
	return true;
}



bool Backend::Run()
{
	const int InitResult = Initialize();
	if (!InitResult)
	{
		spdlog::error("Initialization failed...");
		return false;
	}

	const int UpdateResult = Update();
	if (!UpdateResult)
	{
		spdlog::error("Encountered error during Update()...");
		return false;
	}

	return true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

	glViewport(0, 0, width, height);
}

void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color, float screen_width, float screen_height)
{
	glm::mat4 projection = glm::ortho(0.0f, screen_width, 0.0f, screen_height);

	// activate corresponding render state	
	shader.use();
	shader.setMat4("projection", projection);
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);


		// bitshift by 6 to get value in pixels
		x += (ch.Advance >> 6) * scale;  
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_DEPTH_TEST);
}

void Backend::InitializeUserInterface()
{
	try
	{
		// FreeType
	// --------
		FT_Library ft;
		// All functions return a value different than 0 whenever an error occurred
		if (FT_Init_FreeType(&ft))
		{
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			return;
		}

		if (fontPath.empty())
		{
			std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
			return;
		}

		// load font as face
		FT_Face face;
		if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			return;
		}
		else {
			// set size to load glyphs as
			FT_Set_Pixel_Sizes(face, 0, 48);

			// disable byte-alignment restriction
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// load first 128 characters of ASCII set
			for (unsigned char c = 0; c < 128; c++)
			{
				// Load character glyph 
				if (FT_Load_Char(face, c, FT_LOAD_RENDER))
				{
					std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
					continue;
				}
				// generate texture
				unsigned int texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexImage2D(
					GL_TEXTURE_2D,
					0,
					GL_RED,
					face->glyph->bitmap.width,
					face->glyph->bitmap.rows,
					0,
					GL_RED,
					GL_UNSIGNED_BYTE,
					face->glyph->bitmap.buffer
				);
				// set texture options
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				// now store character for later use
				Character character = {
					texture,
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					static_cast<unsigned int>(face->glyph->advance.x)
				};
				Characters.insert(std::pair<char, Character>(c, character));
			}
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		// destroy FreeType once we're finished
		FT_Done_Face(face);
		FT_Done_FreeType(ft);


		// configure VAO/VBO for texture quads
		// -----------------------------------
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}
	catch (std::exception FailureReason)
	{
		spdlog::error(FailureReason.what());
	}
}

void PollInputs(GLFWwindow* window)
{

	if (camera.GetFreeLook())
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			camera.SetMovementSpeed(camera.BoostSpeed);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, EditorTime.deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, EditorTime.deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, EditorTime.deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, EditorTime.deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
			camera.SetMovementSpeed(camera.SPEED);
	}
	glfwSetKeyCallback(window, Input_Callback);
	
}



void OnMouseMove(double deltaX, double deltaY)
{
	const float sensitivity = 1.1f; 

	if (camera.isOrbiting)
	{
		camera.OrbitAroundTarget(camera.OrbitTarget, 10.0f, sensitivity, deltaX, deltaY);
		//camera.LookAtWithYaw(camera.OrbitTarget);
	}
}
	


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	
	if (EditorWindow.m_SceneHovered)
	{
		yoffset *= camera.m_ZoomScrollFactor;
		camera.ProcessMouseScroll((float)yoffset);
	}

}

float lastX = 200, lastY = 400;
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
float xoffset = 0.0f;
float yoffset = 0.0f;
	static bool firstMouse = true;
	

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}


	xoffset = xpos - lastX;
	yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	
	
	
	
	if (camera.isOrbiting)
	{
		camera.OrbitAroundTarget(camera.OrbitTarget, 5.0f, 0.1f, xoffset, yoffset);
	}

	if (camera.GetFreeLook())
		camera.ProcessMouseMovement(xoffset, yoffset);

}


void OnMouseButton(int button, int action, GLFWwindow* window)
{
	if (EditorWindow.m_SceneHovered)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
			{
				camera.isOrbiting = true;
				//OnMouseMove(glfwGetCursorPos.)
			}
			else if (action == GLFW_RELEASE)
			{
				camera.isOrbiting = false;
			}
		}
	}
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double x, y;

	glfwGetCursorPos(window, &x, &y);

	glm::vec4 viewport = glm::vec4(0, 0, Backend::m_Width, Backend::m_Height);
	glm::vec3 winPos = glm::vec3(x, Backend::m_Height - y, 0.0f); 

	OnMouseButton(button, action, window);

	if (EditorWindow.m_SceneHovered)
	{


		if ((button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) && !ImGui::IsAnyItemHovered())
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			camera.SetFreeLook(true);
		}
		
	}

	 if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		camera.isOrbiting = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		camera.SetFreeLook(false);

	}
}

void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGuizmo::OPERATION tempOp = EditorWindow.myOperation;
	switch (key)
	{
	case GLFW_KEY_W:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.myOperation = ImGuizmo::OPERATION::TRANSLATE;
		}
		break;
	case GLFW_KEY_R:
		
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.myOperation = ImGuizmo::OPERATION::ROTATE;
		}
		
		if (action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL) && !ImGui::IsAnyItemActive())
		{
			EditorWindow.editingName = true;
			EditorWindow.myOperation = tempOp;
		}
		break;
	case GLFW_KEY_T:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.myOperation = ImGuizmo::OPERATION::SCALE;
		}
		break;
		// Enable/disable debug window
	case GLFW_KEY_H:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.DEBUG_MODE = !EditorWindow.DEBUG_MODE;
		}
		break;
		
	case GLFW_KEY_G:
		

		break;
		// Stop the models from spinning 
	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			// Under revision
		}
		break;
		// Fullscreen hotkey
	case GLFW_KEY_F11:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.ToggleFullscreen(window, EditorWindow.myBack);
		}
		break;
		// Stop the models from spinning 
	case GLFW_KEY_F:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.Task_FocusObject();
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
			if (EditorWindow.editingName)
			{
				EditorWindow.editingNameLoggingMsg = fmt::format("{}", EditorWindow.editingNameLoggingMsg);
				EditorWindow.LoggingEntries.push_back(fmt::format("{} to \"{}\" ", EditorWindow.editingNameLoggingMsg, EditorWindow.editingTempName));
				EditorWindow.editingNameLoggingMsg = "";
				EditorWindow.DebugSelectedEntity->Name = EditorWindow.editingTempName;
				EditorWindow.editingName = false;
			}
		}
		break;
	case GLFW_KEY_N:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			if (EditorWindow.DebugSelectedEntity->GetComponent<Model>().GetVisible())
			{
				if (EditorWindow.DebugSelectedEntity->GetComponent<Model>().RenderMode == RENDERTARGETS::NORMAL)
				{
					EditorWindow.DebugSelectedEntity->GetComponent<Model>().RenderMode = RENDERTARGETS::LIT;
				}
				else {
					EditorWindow.DebugSelectedEntity->GetComponent<Model>().RenderMode = RENDERTARGETS::NORMAL;
				}
				
				
			}
			else
			{
				EditorWindow.LoggingEntries.push_back("No object with shader selected!!");
			}
		}
		break;
	case GLFW_KEY_L:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			if (EditorWindow.DebugSelectedEntity != nullptr)
			{
				if (EditorWindow.DebugSelectedEntity->GetComponent<Model>().GetVisible())
				{
					if (EditorWindow.DebugSelectedEntity->GetComponent<Model>().RenderMode == RENDERTARGETS::LINES)
					{
						EditorWindow.DebugSelectedEntity->GetComponent<Model>().RenderMode = RENDERTARGETS::LIT;
					}
					else {
						EditorWindow.DebugSelectedEntity->GetComponent<Model>().RenderMode = RENDERTARGETS::LINES;
					}
				}
			}
			else
			{
				EditorWindow.LoggingEntries.push_back("No object selected!!");
			}
		}
		break;

		case GLFW_KEY_DELETE:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.Task_Delete();
		}
		break;
		// Quit out of program
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.Exit_Application(window);
		}
		break;
	case GLFW_KEY_O:
		if (action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL) && !ImGui::IsAnyItemActive())
		{
			EditorWindow.Task_ImportModel(*EditorWindow.DebugEntityList);
		}
		break;
	case GLFW_KEY_F12:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			system("start https://learnopengl.com/");
		}
		break;
	case GLFW_KEY_X:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.renderUI = !EditorWindow.renderUI;
		}
		break;
	case GLFW_KEY_M:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			camera.mode = static_cast<Camera_Mode>((camera.mode + 1) % 2);	
		}
		break;
	}

}

