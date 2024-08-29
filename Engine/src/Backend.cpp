#include "../Headers/Backend.h"

void PollInputs(GLFWwindow* window);

bool DEBUG_MODE = false;
bool shouldSpin = true;
bool reverseSpin = false;
float deltaTime;
Camera camera;
glm::mat4 Backend::view = glm::mat4(1.0f);       
glm::mat4 Backend::projection = glm::mat4(1.0f);
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
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
		if (action == GLFW_PRESS)
		{
			DEBUG_MODE = !DEBUG_MODE;
		}
		break;
		// Display Position
	case GLFW_KEY_G:
		if (action == GLFW_PRESS)
		{
			std::cout << "Camera position: \nx: " << camera.Position.x << "\ny: " << camera.Position.y << "\nz: " << camera.Position.z << std::endl;
			std::cout << "Camera speed: " << camera.MovementSpeed << std::endl;
		}

		break;
	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS)
		{
			shouldSpin = !shouldSpin;
		}
		break;

	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS)
		{
			std::cout << "Goodbye!" << std::endl;

			glfwSetWindowShouldClose(window, true);
		}
		break;
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

void mouse_button_callback(GLFWwindow* window,  int button, int action, int mods)
{
	double x, y;

	glfwGetCursorPos(window, &x, &y);



	glm::vec4 viewport = glm::vec4(0, 0, Backend::width, Backend::height);
	glm::vec3 winPos = glm::vec3(x, Backend::height - y, 0.0f); // Near plane
	glm::vec3 nearPoint = glm::unProject(winPos, Backend::view, Backend::projection, viewport);
	winPos.z = 1.0f; // Far plane
	glm::vec3 farPoint = glm::unProject(winPos, Backend::view, Backend::projection, viewport);


	


	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		spdlog::warn("\nMouse Position \nX:{}\nY:{}", x, y);
		spdlog::warn("Far Point: \nx: {}\ny: {}\nz: {}\nNear Point: \nx: {}\ny: {}\nz: {}\nWindow Position: \nx: {}\ny: {}\nz: \n", farPoint.x, farPoint.y, farPoint.z, nearPoint.x, nearPoint.y, nearPoint.z, winPos.x, winPos.y, winPos.z);
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

int Backend::Initialize()
{
	deltaTime = 0.0f;
	shouldSpin = true;

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

	// Create GLFW Window 
	window = glfwCreateWindow(width, height, "Engine", NULL, NULL);
	if (!window)
	{
		spdlog::error("Uh oh something went wrong...");
		return 0;
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

	Shader shaders("..\\Engine\\Shaders\\vertex_shader.vert", "..\\Engine\\Shaders\\fragment_shader.frag");
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
	//Model ourModel("../Engine/Models/blockert.fbx");
	//Model ourModel1("../Engine/Models/Table.obj");
	//Model MonkeyMan("../Engine/Models/Monkey.obj");
	//Model Building("../Engine/Models/Test.obj");
	Model Backpack("../Engine/Models/backpack.obj");
	Model Backpack2("../Engine/Models/Blockert.fbx");
	//ModelList.push_back(ourModel);
	//ModelList.push_back(ourModel1);
	//ModelList.push_back(MonkeyMan);
	ModelList.push_back(Backpack);
	ModelList.push_back(Backpack2);
	//ModelList.push_back(Building);

	return 0;
}
int Backend::Update()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Main Loop *CORE*
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); // Start Frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		PollInputs(window);

		//glClearColor(.250f, 0.35f, .35f, 1.0f);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

		float currentFrame = glfwGetTime();  // Get the current time
		deltaTime = currentFrame - lastFrame; // Calculate delta time
		lastFrame = currentFrame;  // Update last frame time

		if (shouldSpin)
		{
			if (!reverseSpin)
				rotationAngle += 0.5f;
			else
				rotationAngle -= .5f;

			if (rotationAngle > 360.0f)
			{
				rotationAngle -= 360.0f;
			}
			if (rotationAngle < 360.0f)
			{
				rotationAngle += 360.0f;
			}
		}

		projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		
		glm::mat4 model = glm::mat4(1.0f);


		for (int i = 0; i < ModelList.size(); i++)
		{
			Model& modelitem = ModelList[i];
			
			//modelitem.Draw(TempShader);
			if (i > 0)
			{
				modelitem.SetPosition(glm::vec3(-2.38f, .92f, 1.9f));
				//modelitem.SetRotation(-rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
				modelitem.SetScale(glm::vec3(.25f, .25f, .25f));

				glm::mat4 model = glm::mat4(1.0f);
				lightCubeShader.setMat4("model", model);

				lightCubeShader.use();
				lightCubeShader.setMat4("projection", projection);
				lightCubeShader.setMat4("view", view);
				

				modelitem.Draw(lightCubeShader);
			}
			else
			{
				modelitem.SetPosition(glm::vec3(-i + .5f, 0.0f, 0.0f));
				modelitem.SetRotation(-rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
				modelitem.SetScale(glm::vec3(.25f, .25f, .25f));

				glm::mat4 model = glm::mat4(1.0f);
				TempShader.setMat4("model", model);

				TempShader.use();
				TempShader.setMat4("projection", projection);
				TempShader.setMat4("view", view);
				TempShader.setVec3("lightPos", modelitem.GetPosition());
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

Backend::Backend()
{
	spdlog::info("Initializing Backend");
}

void Backend::DebugWindow(ImGuiIO& io)
{
	static float scaleX = 1.0f;
	static float scaleY = 1.0f;
	static float scaleZ = 1.0f;

	ImGui::Begin("Debug Window");
	ImGui::Text("Camera:");
	ImGui::Text("X position: %.2f", camera.Position.x);
	ImGui::Text("Y position: %.2f", camera.Position.y);
	ImGui::Text("Z position: %.2f", camera.Position.z);

	ImGui::Checkbox("Reverse Direction", &reverseSpin);
	ImGui::Checkbox("Spin Model", &shouldSpin);

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

	ImGui::Render();

	if (DEBUG_MODE)
	{
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
