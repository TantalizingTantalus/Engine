#include "../Headers/Backend.h"
#include "../Headers/Light.h"

//									 Global variables 



Editor EditorWindow;
Camera camera;
Time EditorTime;

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
	}
	
	// Additional window setup
	{
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

		// Set full height to glfw window
		GLFWmonitor* primMonitor = glfwGetPrimaryMonitor();
		if (primMonitor)
		{
			const GLFWvidmode* videoMode = glfwGetVideoMode(primMonitor);
			if (videoMode)
			{
				full_height = videoMode->height;
				full_width = videoMode->width;
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
		Shader stencilShader("../Engine/Shaders/LitMaterial_Shader.vert", "../Engine/Shaders/shaderSingleColor.frag");
		Shader myTextShader("../Engine/Shaders/TextGlyph.vert", "../Engine/Shaders/TextGlyph.frag");
		TempShader = shaders;
		lightCubeShader = lightShader;
		textShader = myTextShader;
	}
	
	// Initialize IMGUI
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}
	
	// Setup glfw input Callbacks
	{
		glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
		glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
		glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
		glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
	}

	// Component Initialization
	{
		/*std::shared_ptr<Light> light_Component = std::make_shared<Light>();
		std::shared_ptr<Transform> transform_Component = std::make_shared<Transform>();
		std::shared_ptr<TestComponent> test_Component = std::make_shared<TestComponent>();

		*/
	}

	{
		// Create default light model
		std::string dLight = "LightSource";
		Model LightSourceModelComp("../Engine/Models/Light_Cube.fbx");
		Entity LightSourceEnt(dLight.c_str());
		LightSourceEnt.transform->setLocalScale(glm::vec3(0.25f, 0.25f, 0.25f));
		// LightSourceObj.UpdateModelMatrix();
		LightSourceModelComp.IsLight = true;
		std::shared_ptr<Model> LightSourceModel = std::make_shared<Model>(LightSourceModelComp);
		//LightSourceModel->SetParent(&LightSourceEnt);
		LightSourceEnt.AddComponent(LightSourceModel);
		LightSourceEnt.GetComponent<Model>().SetVisible(false);
		LightSourceEnt.GetComponent<Model>().parentEntity = &LightSourceEnt;
		

		// Add components to model
		
		LightSourceEnt.AddComponent(LightSourceEnt.transform);
		std::shared_ptr<Light> light_Component = std::make_shared<Light>();
		//std::shared_ptr<Light> mp = componentLibrary.GetComponent<Light>();

		LightSourceEnt.AddComponent(light_Component);

		// Ship it
		LightSourceEnt.GetComponent<Model>().parentEntity = &LightSourceEnt;
		ModelList.push_back(LightSourceEnt);

		// Create default room model
		Model Room("../Engine/Models/Room.fbx");
		Entity RoomEnt(Room.GetModelName().c_str());
		// Add components to model
		std::shared_ptr<Model> RoomModelComp = std::make_shared<Model>(Room);
		RoomEnt.AddComponent(RoomEnt.transform);
		RoomEnt.AddComponent(RoomModelComp);
		RoomEnt.GetComponent<Model>().parentEntity = &RoomEnt;

		// Ship it
		ModelList.push_back(RoomEnt);
	}

	// Position default render list (mostly unused unless for a demo)
	{
		for (int i = 0; i < ModelList.size(); i++)
		{
			Entity& modelitem = ModelList[i];
			modelitem.transform->setLocalPosition(glm::vec3(-i + .5f, 0.0f, 0.0f));
		}
	}

	// Auto select the first item in the render list for manipulation.
	if (ModelList.size() > 0)
		EditorWindow.DebugSelectedEntity = &ModelList[0];

	//Initialize camera after window creation to update framebuffersize for imguizmo
	camera.Initialize(window);
	
	camera.Position = glm::vec3(.6f, .83f, 1.3f);
	camera.Yaw = -134.6f;
	camera.Pitch = -27.0f;
	camera.updateCameraVectors();

	// return 1 for complete inits
	return 1;
}

int Backend::Update()
{
	EditorWindow.Task_LoadDefaultLayout();
	// Last minute ImGui io setup
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), editor_fontSize);
	

	ImGuiStyle& Style = ImGui::GetStyle();
	Style.WindowMenuButtonPosition = ImGuiDir_None;


	InitializeUserInterface();

	LoadEngineIcon();

	FrameBuffer sceneBuf(width, height);

	EditorWindow.Init(GetBackEnd());
	// Main Loop *CORE*
	while (!glfwWindowShouldClose(window))
	{

		glfwPollEvents(); // Start Frame
		PollInputs(window);

		// Begin ImGui Inits
		StartImGui();

		// Bind framebuffer
		sceneBuf.Bind();

		// Docking space for ImGui setup
		UpdateDockingScene();

		// Begin Scene window frame
		ImGui::Begin("Scene", nullptr);

		// Size scene to ImGui window
		SceneWidth = ImGui::GetContentRegionAvail().x;
		SceneHeight = ImGui::GetContentRegionAvail().y;

		

		// Framebuffer setup - for some reason I haven't made time to investigate this
		// will only work in this update while loop. I've only tried encapsulating in a 
		// method to no avail. Look into Framebuffers and their scopes involving encapsulation.
		glViewport(0, 0, (GLsizei)SceneWidth, (GLsizei)SceneHeight);

		
		ImGui::Image(
			(ImTextureID)sceneBuf.getFrameTexture(),
			ImGui::GetContentRegionAvail(),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);

		sceneBuf.RescaleFrameBuffer(SceneWidth, SceneHeight);

		// Time tracking
		EditorTime.Update();

		// Editor window setup
		EditorWindow.WindowUpdate(camera, *window);

		// Render ModelList
		RenderModels();

		// Begin Game UI
		RenderUI();

		// End Render scene
		ImGui::End();

		// Unbind the scene buffer
		sceneBuf.Unbind();

		// Send io for editor menus
		EditorWindow.DebugWindow(io, ModelList);

		// Final
		glfwSwapBuffers(window);
	}

	// Cleanup
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
	ImGui::SetNextWindowSize(ImVec2(float(GetWindowWidth(window)), float(GetWindowHeight(window))));
	ImGui::Begin("Engine", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0, 0));
	ImGui::DockSpace(ImGui::GetID("Dockspace1"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

	return true;
}

bool Backend::LoadEngineIcon()
{
	// Load the image
	int width, height, channels;
	unsigned char* data = stbi_load("../Engine/src/icon.png", &width, &height, &channels, 4);
	if (!data) {
		spdlog::error("ran into issues loading window icon...");
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	// Create the glfwImage
	GLFWimage icon;
	icon.width = width;
	icon.height = height;
	icon.pixels = data;

	// Set the window icon
	glfwSetWindowIcon(window, 1, &icon);

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
		RenderText(textShader, fmt::format("Time: {:.2f}", EditorTime.currentFrame), (windowSize.x / windowSize.x) + 25.0f, windowSize.y - 40.0f, .85f, glm::vec3(0.5, 0.8f, 0.2f), windowSize.x, windowSize.y);
		RenderText(textShader, fmt::format("Money: {:.2f}", money), windowSize.x - (windowSize.x / 3) - 25.0f, windowSize.y - 40.0f, .85f, glm::vec3(0.5, 0.8f, 0.2f), windowSize.x, windowSize.y);
	}

	
	return true;
}

bool Backend::RenderModels()
{
	try
	{
		// Update camera matrices every frame
		camera.UpdateViewAndProjectionMatrices();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(EditorWindow.clear_color.x, EditorWindow.clear_color.y, EditorWindow.clear_color.z, EditorWindow.clear_color.w);

		// Begin Gizmo frame
		ImGuizmo::BeginFrame();

		// Gizmo Manipulation
		if (EditorWindow.DebugSelectedEntity != nullptr)
		{
			

			if (camera.mode == Camera_Mode::ORTHO)
				ImGuizmo::SetOrthographic(true);
			else
				ImGuizmo::SetOrthographic(false);
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

		// Render Models
		if (!ModelList.empty())
		{
			ImVec4* DirectionalColor = nullptr;
			
			// Render lights first, models second
			for (int i = 0; i < ModelList.size(); i++) {
				Entity& modelItem = ModelList[i];
				Model& modelitem = modelItem.GetComponent<Model>();
				if (modelitem.IsLight) {
					EditorWindow.DirectionalLightObject = &ModelList[i];

					// Shader setup for the light objects
					lightCubeShader.use();
					lightCubeShader.setMat4("model", modelItem.transform->m_modelMatrix);
					lightCubeShader.setMat4("projection", camera.GetProjectionMatrix());
					lightCubeShader.setMat4("view", camera.GetViewMatrix());
					DirectionalColor = &modelItem.GetComponent<Light>().LightColor;
					modelitem.SetShader(lightCubeShader);

					// Draw light objects *** Do we even need to draw lights?
					modelitem.Draw();		// error here
				}
				else
				{
					// Render Models second
					if (!modelitem.IsLight && modelitem.GetVisible()) {

						// Shader setup for lit models
						TempShader.use();
						/*if (modelitem.parent)
							modelitem.forceUpdateSelfAndChild();*/
						TempShader.setMat4("model", modelItem.transform->m_modelMatrix);
						TempShader.setMat4("projection", camera.GetProjectionMatrix());
						TempShader.setMat4("view", camera.GetViewMatrix());
						TempShader.setVec3("lightPos", EditorWindow.DirectionalLightObject->GetComponent<Transform>().getLocalPosition());
						TempShader.setVec3("viewPos", camera.Position);

						TempShader.setVec3("lightColor", glm::vec3(DirectionalColor->x, DirectionalColor->y, DirectionalColor->z));
						
						modelitem.SetShader(TempShader);
						// Draw the model item, this is 1 draw call per frame 60fps = 60 draw calls
						modelitem.Draw();
					}
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
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

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

void InitializeUserInterface()
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

	// find path to font
	std::string font_name = "C:\\Users\\Gaevi\\OneDrive\\Documents\\CodePractice\\Engine\\Engine\\Engine\\Fonts\\arial.ttf";
	if (font_name.empty())
	{
		std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
		return;
	}

	// load font as face
	FT_Face face;
	if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
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
		// to do handle left click 
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
			EditorWindow.LoggingEntries.push_back(s);
		}
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
		// Hotkey to re-locate directional light
	case GLFW_KEY_G:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			EditorWindow.Task_AlignDirLight();
		}

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
				EditorWindow.DebugSelectedEntity->GetComponent<Model>().SetModelName(EditorWindow.editingTempName);
				EditorWindow.editingName = false;
			}
		}
		break;
	case GLFW_KEY_N:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			if (EditorWindow.DebugSelectedEntity->GetComponent<Model>().GetVisible())
			{
				EditorWindow.DebugSelectedEntity->GetComponent<Model>().RenderMode = RENDERTARGETS::NORMAL;
				EditorWindow.Task_DebugNormals(EditorWindow.DEBUG_NORMAL_MAP, EditorWindow.DebugSelectedEntity->GetComponent<Model>().GetShaderID());
				EditorWindow.LoggingEntries.push_back(fmt::format("Show normals: {}", EditorWindow.DEBUG_NORMAL_MAP));
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
				EditorWindow.DebugSelectedEntity->GetComponent<Model>().RenderMode = RENDERTARGETS::LINES;

				EditorWindow.LoggingEntries.push_back(fmt::format("Rendering lines"));
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

