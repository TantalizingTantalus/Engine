#include "../Headers/Backend.h"

Editor EditorWindow;

struct Character {
	unsigned int TextureID; 
	glm::ivec2   Size;      
	glm::ivec2   Bearing;   
	unsigned int Advance;   
};

std::map<GLchar, Character> Characters;
unsigned int VAO, VBO;

void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color, float, float);

Backend::Backend()
{
	spdlog::info("Initializing Backend");
}

Backend::~Backend()
{
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

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

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

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
		glfwSwapInterval(0);

		//Lock cursor to window
		if (camera.GetFreeLook())
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		

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
		spdlog::info("GLFW Version : {}", glfwGetVersionString());	
		const GLubyte* glVersion = glGetString(GL_VERSION);
		const GLubyte* glRenderer = glGetString(GL_RENDERER);
		spdlog::info("OpenGL Version: {}", reinterpret_cast<const char*>(glVersion));
		spdlog::info("Renderer: {}", reinterpret_cast<const char*>(glRenderer));
	}

	// Shader Setup
	{
		Shader shaders("Shaders/LitMaterial_Shader.vert", "Shaders/LitMaterial_Shader.frag");
		Shader lightShader("Shaders/lightSource.vert", "Shaders/lightSource.frag");
		
		Shader myTextShader("Shaders/TextGlyph.vert", "Shaders/TextGlyph.frag");
		Shader shadShader("Shaders/ShadowMapShader.vert", "Shaders/EmptyFragment.frag");

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
		ImGui_ImplGlfw_InitForOpenGL(m_Window, false);
		
		ImGui_ImplOpenGL3_Init("#version 410");
		ImGui_ImplGlfw_InstallCallbacks(m_Window);
	}
	
	// Setup glfw input Callbacks - disabled for now for default callback handling, enabling this requires
	// custom backspace, tab, and carriage return functionality which I do not care to implement right now.
	{
		/*glfwSetCharCallback(m_Window, ImGui_ImplGlfw_CharCallback);
		glfwSetKeyCallback(m_Window, ImGui_ImplGlfw_KeyCallback);
		glfwSetMouseButtonCallback(m_Window, ImGui_ImplGlfw_MouseButtonCallback);
		glfwSetScrollCallback(m_Window, ImGui_ImplGlfw_ScrollCallback);*/
		//glfwSetCursorPosCallback(m_Window, mouse_callback);
	}

	// Auto select the first item in the render list for manipulation.
	if (m_ModelMap.size() > 0)
		SelectEntity(0);


	camera.Initialize(m_Window);
	
	camera.Position = glm::vec3(.6f, .83f, 1.3f);

	// return 1 for complete inits
	return 1;
}

int Backend::Update()
{
	// This loads the window layout from default file
	std::ifstream src("DefaultLayout.ini", std::ios::binary);
	std::ofstream dst("imgui.ini", std::ios::binary);

	std::filesystem::path defaultLayoutPath = "DefaultLayout.ini";
	std::filesystem::path imGuiLayoutPath = "imgui.ini";

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
	GLuint PickingTexture = sceneBuf.SetupMousePicking(m_Width, m_Height);

	Shader stencilShader("Shaders/shaderSingleColor.vert", "Shaders/shaderSingleColor.frag");
	m_StencilShader = stencilShader;

	
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
	//glDrawBuffer(GL_NONE); 
	//glReadBuffer(GL_NONE);

	// //Check completeness
	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	//	spdlog::error("Framebuffer not complete!\n");
	//}

	// Unbind Framebuffer
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Scene vars
	int prevSceneWidth = 0, prevSceneHeight = 0;

	// Initialize the imgui editor windows
	EditorWindow.Init(GetBackEnd());


	// project explorer loader? Nested for loops bad practice
	{
		for (auto& p : std::filesystem::directory_iterator(EditorWindow.myPath))
		{
			if (p.exists())
			{
				if (!p.path().empty())
				{
					for (auto& t : std::filesystem::directory_iterator(p))
					{
						if (t.path().extension() == ".fbx" || t.path().extension() == ".obj")
						{
							Model t_LoadedModel(t.path().string());
							Entity E_LoadedModel(t_LoadedModel.GetModelName().c_str());
							Material Mat_LoadedModel(&E_LoadedModel);

							std::shared_ptr<Model> t_LoadedModelComp = std::make_shared<Model>(t_LoadedModel);
							std::shared_ptr<Material> Mat_LoadedModelComp = std::make_shared<Material>(Mat_LoadedModel);

							E_LoadedModel.AddComponent(E_LoadedModel.transform);
							E_LoadedModel.AddComponent(t_LoadedModelComp);
							E_LoadedModel.AddComponent(Mat_LoadedModelComp);
							E_LoadedModel.GetComponent<Model>().parentEntity = &E_LoadedModel;
							E_LoadedModel.GetComponent<Material>().Initialize(E_LoadedModel.GetComponent<Model>());
							E_LoadedModel.GetComponent<Material>().pModel->SetShader(m_LitMaterialShader);
							E_LoadedModel.ID = m_LoadedModelsList.size() + 1;

							m_LoadedModelsList.push_back(E_LoadedModel);

						}
					}
				}
			}
		}
	}


	// Pre-Load directional light
	m_LitMaterialShader.use();
	m_LitMaterialShader.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
	m_LitMaterialShader.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
	m_LitMaterialShader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	m_LitMaterialShader.setFloat("dirLight.intensity", MyDirLight.m_Intensity);
	m_LitMaterialShader.setBool("dirLight.inUse", MyDirLight.isActive);
	MyDirLight.m_DirShader = &m_LitMaterialShader;
	
	// Handle point lights
	for (auto& light : m_PointLightsEntityMap)
	{
		if (light.second.HasComponent<Light>())
		{
			// to fix... init values for light component to default due to default objects being destructed during Backend::Init()...
			light.second.GetComponent<Light>().lightIntensity = 1.0f;
			light.second.GetComponent<Light>().LightColor = ImVec4(1, 0, 0, 1);
			m_LitMaterialShader.use();
			m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].ambient", light.first), 0.05f, 0.05f, 0.05f);
			m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].diffuse", light.first), 0.8f, 0.8f, 0.8f);
			m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].specular", light.first), 1.0f, 1.0f, 1.0f);
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].intensity", light.first), light.second.GetComponent<Light>().lightIntensity);
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].constant", light.first), 1.0f);
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].linear", light.first), 0.09f);
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].quadratic", light.first), 0.032f);
		}
	}

	// Initialize icons used by the renderer/editor
	SystemIcons::Initialize();

	// Main Loop *CORE*
	while (!glfwWindowShouldClose(m_Window))
	{
		// Start Frame
		glfwPollEvents();
		PollInputs(m_Window);
		EditorWindow.PollEditorInput(m_Window);

		// Handle directional light
		m_LitMaterialShader.use();
		m_LitMaterialShader.setVec3("dirLight.direction", MyDirLight.m_Direction);
		m_LitMaterialShader.setVec3("dirLight.color", glm::vec3(MyDirLight.m_Color.x, MyDirLight.m_Color.y, MyDirLight.m_Color.z));
		m_LitMaterialShader.setFloat("dirLight.intensity", MyDirLight.m_Intensity);

		// Handle point lights
		for (auto& light : m_PointLightsEntityMap)
		{
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].intensity", light.first), light.second.GetComponent<Light>().lightIntensity);
			m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].position", light.first), light.second.GetComponent<Transform>().position);
			m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].color", light.first), glm::vec3(light.second.GetComponent<Light>().LightColor.x, light.second.GetComponent<Light>().LightColor.y, light.second.GetComponent<Light>().LightColor.z));
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].constant", light.first), 1.0f);
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].linear", light.first), 0.09f);
			m_LitMaterialShader.setFloat(fmt::format("pointLights[{}].quadratic", light.first), 0.032f);
		}

		// Begin ImGui Inits
		StartImGui();
		// Bind framebuffer
		sceneBuf.Bind();

		// Docking space for ImGui setup
		UpdateDockingScene();
		

		// Begin Scene window frame
		ImGui::Begin("Scene", nullptr);
		EditorWindow.m_SceneWindowPosition = ImGui::GetWindowPos();

		if (ImGui::IsWindowHovered())
		{
			EditorWindow.m_SceneHovered = true;
		}
		else
		{
			EditorWindow.m_SceneHovered = false;
		}

		// Size scene to ImGui window
		m_SceneWidth = ImGui::GetContentRegionAvail().x;
		m_SceneHeight = ImGui::GetContentRegionAvail().y;

		glfwGetCursorPos(m_Window, &EditorWindow.MouseX, &EditorWindow.MouseY);
		

		// If screen is resized, update the picking texture
		if (m_SceneWidth != prevSceneWidth || m_SceneHeight != prevSceneHeight) {
			glBindTexture(GL_TEXTURE_2D, PickingTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_SceneWidth, m_SceneHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);

			sceneBuf.RescaleFrameBuffer(m_SceneWidth, m_SceneHeight);
			prevSceneWidth = m_SceneWidth;
			prevSceneHeight = m_SceneHeight;

			spdlog::info(fmt::format("Screen resized to: \nX: {}\nY: {}", m_SceneWidth, m_SceneHeight));
		}

		glViewport(0, 0, (GLsizei)m_SceneWidth, (GLsizei)m_SceneHeight);

		// To do: Implement Depth Map (Shadow maps)
		// 
		//  first render to depth map
		//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		//glClear(GL_DEPTH_BUFFER_BIT);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//// then render scene as normal with shadow mapping 
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
		sceneBuf.Unbind();

		// Send io for editor menus
		EditorWindow.DebugWindow(io, m_ModelMap);

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
	unsigned char* data = stbi_load("icon.png", &width, &height, &channels, 4);
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
	for (auto& ent : m_ModelMap)
	{
		if (ent.first == id)
		{
			EditorWindow.DebugSelectedEntity = &m_ModelMap[id];
			if (EditorWindow.camera)
			{
				EditorWindow.camera->OrbitTarget = m_ModelMap[id].GetComponent<Transform>().position;
			}
		}
		else
		{
			EditorWindow.DebugSelectedEntity = &m_ModelMap.begin()->second;
			if (EditorWindow.camera && m_ModelMap.begin()->second.HasComponent<Transform>())
			{
				
				EditorWindow.camera->OrbitTarget = m_ModelMap.begin()->second.GetComponent<Transform>().position;
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

		//EditorWindow.UpdateEntities(m_ModelMap);    <---- why is this commented?
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glClearColor(EditorWindow.clear_color.x, EditorWindow.clear_color.y, EditorWindow.clear_color.z, EditorWindow.clear_color.w);

		

		// Begin Gizmo frame
		ImGuizmo::BeginFrame();

		// Gizmo Manipulation
		if (EditorWindow.DebugSelectedEntity != nullptr)
		{
			switch (camera.mode)
			{
			case Camera_Mode::ORTHO:
				ImGuizmo::SetOrthographic(true);
				break;
			case Camera_Mode::PERSPECTIVE:
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
				EditorWindow.EditorTransformationOperation,
				ImGuizmo::LOCAL,
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

		for (auto& m_Model : m_ModelMap)
		{
			if (m_Model.second.HasComponent<Light>() )
			{
				if (m_Model.second.GetComponent<Light>().lightType == LightType::POINTLIGHT)
				{
					m_PointLightsEntityMap.emplace(m_Model.first, m_Model.second);
				}
			}
		}

		// Render Models
		if (!m_ModelMap.empty())
		{
			// Render lights first, models second
			for (auto& light : m_PointLightsEntityMap)
			{
				m_LitMaterialShader.use();
				m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].position", light.first), light.second.GetComponent<Transform>().position);
				m_LitMaterialShader.setVec3(fmt::format("pointLights[{}].color", light.first), glm::vec3(light.second.GetComponent<Light>().LightColor.x, light.second.GetComponent<Light>().LightColor.y, light.second.GetComponent<Light>().LightColor.z));
			}

			ImVec4* DirectionalColor = nullptr;
			
			for (auto& modelItem : m_ModelMap)
			{

				modelItem.second.GetComponent<Model>().SetShader(m_LightShader);
				if (modelItem.second.HasComponent<Light>()) {

					// Shader setup for the light objects
					m_LightShader.use();
					m_LightShader.setMat4("model", modelItem.second.GetComponent<Transform>().m_modelMatrix);
					m_LightShader.setMat4("projection", camera.GetProjectionMatrix());
					m_LightShader.setMat4("view", camera.GetViewMatrix());
					m_LightShader.setInt("entityID", modelItem.first);
					DirectionalColor = &modelItem.second.GetComponent<Light>().LightColor;

					modelItem.second.GetComponent<Model>().SetShader(m_LightShader);

					// Draw light objects
					modelItem.second.GetComponent<Model>().Draw();
				}
				else
				{
					// Render Models second
					if (!modelItem.second.HasComponent<Light>() && modelItem.second.GetComponent<Model>().GetVisible()) {
						glEnable(GL_STENCIL_TEST);
						glStencilFunc(GL_ALWAYS, 1, 0xFF);
						glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
						glStencilMask(0xFF);
						glDepthMask(GL_TRUE);
						glEnable(GL_DEPTH_TEST);
						modelItem.second.GetComponent<Model>().SetShader(m_LitMaterialShader);

						// Shader setup for lit models
						m_LitMaterialShader.use();

						m_LitMaterialShader.setMat4("model", modelItem.second.transform->m_modelMatrix);
						m_LitMaterialShader.setMat4("projection", camera.GetProjectionMatrix());
						m_LitMaterialShader.setMat4("view", camera.GetViewMatrix());
						m_LitMaterialShader.setVec3("viewPos", camera.Position);
						if (modelItem.second.HasComponent<Material>())
						{
							m_LitMaterialShader.setBool("material.hasSpecular", modelItem.second.GetComponent<Model>().hasSpecular);
							m_LitMaterialShader.setBool("material.hasNormal", modelItem.second.GetComponent<Model>().hasNormal);
							m_LitMaterialShader.setFloat("material.specularIntensity", modelItem.second.GetComponent<Material>().m_SpecIntensity);
							m_LitMaterialShader.setFloat("material.shininess", modelItem.second.GetComponent<Material>().m_Shininess);
						}

						m_LitMaterialShader.setInt("entityID", modelItem.first);

						// Draw the model item, this is 1 draw call per frame 60fps = 60 draw calls
						modelItem.second.GetComponent<Model>().Draw();


						if (modelItem.first == EditorWindow.DebugSelectedEntity->ID && !EditorWindow.DEBUG_NORMAL_MAP)
						{

							glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
							glStencilMask(0x00);

							m_StencilShader.use();
							glm::mat4 scaledModelMatrix = glm::scale(
								modelItem.second.transform->m_modelMatrix,
								glm::vec3(EditorWindow.OutlineThickness, EditorWindow.OutlineThickness, EditorWindow.OutlineThickness)
							);
							m_StencilShader.setMat4("model", scaledModelMatrix);
							m_StencilShader.setMat4("projection", camera.GetProjectionMatrix());
							m_StencilShader.setMat4("view", camera.GetViewMatrix());
							m_StencilShader.setVec3("viewPos", camera.Position);
							m_StencilShader.setInt("entityID", modelItem.first);
							
							modelItem.second.GetComponent<Model>().SetShader(m_StencilShader);
							modelItem.second.GetComponent<Model>().Draw();

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
		EditorWindow.m_DebugEntityMap = &m_ModelMap;

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

bool Backend::InitializeFreeType(const std::string& fontPath) {
	// Initialize FreeType library
	if (FT_Init_FreeType(&ft)) {
		spdlog::error("ERROR::FREETYPE: Could not init FreeType Library");
		return false;
	}

	// Load font as face
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
		spdlog::error("ERROR::FREETYPE: Failed to load font");
		return false;
	}

	return true;
}

bool Backend::UpdateFontSize(float fontSize)
{
	spdlog::info(fmt::format("updating to x = {}, and y = {}", fontSize, fontSize));
	if (!face) {
		if (!InitializeFreeType(fontPath)) {
			return false;
		}
	}
	FT_Set_Pixel_Sizes(face, fontSize, fontSize);

	// Clear existing characters
	Characters.clear();

	// Load new glyphs
	for (unsigned char c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
			continue;
		}

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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<unsigned int>(face->glyph->advance.x)
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void Backend::InitializeUserInterface()
{
	try
	{
		// FreeType
	// --------
		

		if (FT_Init_FreeType(&ft))
		{
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			return;
		}

		if (fontPath.empty())
		{
			std::cout << "ERROR::FREETYPE: Failed to load font path" << std::endl;
			return;
		}

		FT_Face face;
		if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			return;
		}
		else {
			FT_Set_Pixel_Sizes(face, m_CurrentUI_FontSize, m_CurrentUI_FontSize); // Size creation of UI Text

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			
			for (unsigned char c = 0; c < 128; c++)
			{
				if (FT_Load_Char(face, c, FT_LOAD_RENDER))
				{
					std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
					continue;
				}
				
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
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				
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

void Backend::PollInputs(GLFWwindow* window)
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
	//glfwSetKeyCallback(window, Input_Callback);
	
}

void Backend::PollMouseMovement(float xpos, float ypos)
{
	
	
	if (m_FirstMouseSceneClick) {
		lastX = xpos;
		lastY = ypos;
		m_FirstMouseSceneClick = false;
		return;
	}


	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;


	if (camera.GetFreeLook())
	{
		if (std::abs(xoffset) > 0.1f || std::abs(yoffset) > 0.1f)
			camera.ProcessMouseMovement(xoffset, yoffset);
	}

}
