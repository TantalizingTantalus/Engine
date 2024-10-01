#include "../Headers/Backend.h"


//																							Backend Global variables 
bool DEBUG_MODE = true;
bool DEBUG_NORMAL_MAP = false;
bool ContextMenuActive = false;
bool shouldSpin = false;
bool reverseSpin = false;
bool editingName = false;
bool renderUI = true;
float deltaTime;

Editor EditorWindow;
Camera camera;
ImGuizmo::OPERATION myOperation = ImGuizmo::OPERATION::TRANSLATE;
std::string editingTempName, editingNameLoggingMsg;


struct Character {
	unsigned int TextureID; 
	glm::ivec2   Size;      
	glm::ivec2   Bearing;   
	unsigned int Advance;   
};

std::map<GLchar, Character> Characters;
unsigned int VAO, VBO;

//																								 Function protos

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
	deltaTime = 0.0f;
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

	// Initialize FreeType
	{
		// To implement
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
		Shader shaders("..\\Engine\\Shaders\\LitMaterial_Shader.vert", "..\\Engine\\Shaders\\LitMaterial_Shader.frag");
		Shader lightShader("..\\Engine\\Shaders\\lightSource.vert", "..\\Engine\\Shaders\\lightSource.frag");
		Shader stencilShader("..\\Engine\\Shaders\\LitMaterial_Shader.vert", "..\\Engine\\Shaders\\shaderSingleColor.frag");
		Shader myTextShader("..\\Engine\\Shaders\\TextGlyph.vert", "..\\Engine\\Shaders\\TextGlyph.frag");
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

	// Load default light 
	{
		Model LightSourceObj("../Engine/Models/Light_Cube.fbx");

		LightSourceObj.SetScale(glm::vec3(0.25f, 0.25f, 0.25f));
		//LightSourceObj.UpdateModelMatrix();
		LightSourceObj.IsLight = true;
		LightSourceObj.SetVisible(false);
		std::string dLight = "LightSource";
		LightSourceObj.SetModelName(dLight);
		ModelList.push_back(LightSourceObj);

		Model Room("../Engine/Models/Room.fbx");
		Room.SetRotation(-90, glm::vec3(1, 0, 0));
		ModelList.push_back(Room);
	}

	// Position default render list (mostly unused unless for a demo)
	{
		for (int i = 0; i < ModelList.size(); i++)
		{
			Model& modelitem = ModelList[i];
			modelitem.SetPosition(glm::vec3(-i + .5f, 0.0f, 0.0f));
			modelitem.UpdateModelMatrix();
		}
	}
	

	// Auto select the first item in the render list for manipulation.
	if (ModelList.size() > 0)
		selectedDebugModelIndex = 0;


	//Initialize camera after window creation to update framebuffersize for imguizmo
	camera.Initialize(window);
	

	camera.Position = glm::vec3(.6f, .83f, 1.3f);
	camera.Yaw = -134.6f;
	camera.Pitch = -27.0f;
	camera.updateCameraVectors();

	// return 0 for complete inits
	return 0;
}

int Backend::Update()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	InitializeUserInterface();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	int diW, diH;
	glfwGetFramebufferSize(window, &diW, &diH);

	FrameBuffer sceneBuf(width, height);
	float money = 0, currentTime = 0;

	

	// Main Loop *CORE*
	while (!glfwWindowShouldClose(window))
	{
		
		glfwPollEvents(); // Start Frame
		PollInputs(window);

		// Begin ImGui Inits
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind framebuffer
		sceneBuf.Bind();
		
		// Docking space for ImGui setup
		ImGui::DockSpace(ImGui::GetID("Dockspace1"), ImVec2(0, 0),ImGuiDockNodeFlags_PassthruCentralNode);
		ImGuiWindowFlags hiddenWindowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_MenuBar;

		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		
		// Update camera matrices everyframe
		camera.UpdateViewAndProjectionMatrices();

		// More ImGui setup
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(float(GetWindowWidth(window)), float(GetWindowHeight(window))));
		ImGui::Begin("Engine", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar );
		ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0, 0));

		// Begin Scene window frame
		ImGui::Begin("Scene");
		float winwidth = ImGui::GetContentRegionAvail().x;
		float winheight = ImGui::GetContentRegionAvail().y;

		// Clear viewport and set selected color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(EditorWindow.clear_color.x, EditorWindow.clear_color.y, EditorWindow.clear_color.z, EditorWindow.clear_color.w);

		// Framebuffer setup
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
		// End Imgui Inits

		// Time tracking
		float currentFrame = glfwGetTime();  
		deltaTime = currentFrame - lastFrame; 
		lastFrame = currentFrame;  
		currentTime += deltaTime;

		// Begin Gizmo frame
		ImGuizmo::BeginFrame();

		EditorWindow.Initialize(camera, *window);

		// Gizmo Manipulation
		if (EditorWindow.DebugSelectedObj != nullptr)
		{
			if (camera.mode == Camera_Mode::ORTHO)
				ImGuizmo::SetOrthographic(true);
			else
				ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
			
			glm::mat4* modelMatrix = &EditorWindow.DebugSelectedObj->GetModelMatrix();
			ImGuizmo::Manipulate(glm::value_ptr(camera.GetViewMatrix()), glm::value_ptr(camera.GetProjectionMatrix()),
				myOperation,
				ImGuizmo::LOCAL,
				glm::value_ptr(*modelMatrix));
		}

		// Begin Main render loop
		if (!ModelList.empty())
		{
			// Render lights first, models second
			for (int i = 0; i < ModelList.size(); i++) {
				Model& modelitem = ModelList[i];
				
				if (modelitem.IsLight) {
					EditorWindow.DirectionalLightObject = &modelitem;

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
					// Render Models second
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
						TempShader.setVec3("lightPos", EditorWindow.DirectionalLightObject->GetPosition());
						TempShader.setVec3("viewPos", camera.Position);
						TempShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
						TempShader.setVec3("objectColor", 1.0f, .5f, .31f);
						modelitem.SetShader(TempShader);

						// Draw the model item, this is 1 draw call per frame 60fps = 60 draw calls
						modelitem.Draw();
					}
				}
			}
		}

		// Begin Game UI
		{

			if (renderUI)
			{
				RenderText(textShader, fmt::format("Time: {:.2f}", currentFrame), (windowSize.x / windowSize.x) + 25.0f, windowSize.y - 40.0f, .85f, glm::vec3(0.5, 0.8f, 0.2f), windowSize.x, windowSize.y);
				RenderText(textShader, fmt::format("Money: {:.2f}", money), windowSize.x - (windowSize.x / 3) - 25.0f,windowSize.y - 40.0f, .85f, glm::vec3(0.5, 0.8f, 0.2f), windowSize.x, windowSize.y);
			}
		}

		// Money increment logic
		if (currentTime >= 5.0f) {
			money += 15;
			currentTime -= 5.0f;
		}

		// Set debug modellist to current frame's model list
		EditorWindow.DebugModelList = &ModelList;

		// End Render scene
		ImGui::End();

		// Unbind the scene buffer
		sceneBuf.Unbind();
		
		// Setup io for debug menus
		EditorWindow.DebugWindow(io, ModelList);

		// rescale frame buffer 
		sceneBuf.RescaleFrameBuffer(winwidth, winheight);

		// Final
		glfwSwapBuffers(window); 
	}
	
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	// Return out
	return 0;
}

 //																									Definitions

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
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
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
			EditorWindow.LoggingEntries.push_back(s);
		}
	}
}

void Input_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGuizmo::OPERATION tempOp = myOperation;
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
		
		if (action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL) && !ImGui::IsAnyItemActive())
		{
			editingName = true;
			myOperation = tempOp;
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
			EditorWindow.Task_AlignDirLight();
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
			EditorWindow.ToggleFullscreen(window);
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
			if (editingName)
			{
				editingNameLoggingMsg = fmt::format("{}", editingNameLoggingMsg);
				EditorWindow.LoggingEntries.push_back(fmt::format("{} to \"{}\" ", editingNameLoggingMsg, editingTempName));
				editingNameLoggingMsg = "";
				EditorWindow.DebugSelectedObj->SetModelName(editingTempName);
				editingName = false;
			}
		}
		break;
	case GLFW_KEY_N:
		if (action == GLFW_PRESS && !ImGui::IsAnyItemActive())
		{
			if (EditorWindow.DebugSelectedObj != nullptr)
			{
				EditorWindow.DebugSelectedObj->RenderMode = RENDERTARGETS::NORMAL;
				EditorWindow.Task_DebugNormals(DEBUG_NORMAL_MAP, EditorWindow.DebugSelectedObj->GetShaderID());
				EditorWindow.LoggingEntries.push_back(fmt::format("Show normals: {}", DEBUG_NORMAL_MAP));
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
			if (EditorWindow.DebugSelectedObj != nullptr)
			{
				EditorWindow.DebugSelectedObj->RenderMode = RENDERTARGETS::LINES;

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
			EditorWindow.Task_ImportModel(*EditorWindow.DebugModelList);
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
			renderUI = !renderUI;
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

