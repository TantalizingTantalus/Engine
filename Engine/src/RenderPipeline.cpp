#include "../Headers/RenderPipeline.h"

RenderPipeline::RenderPipeline()
{
	std::cout << "Initializing render pipeline..." << std::endl;
	
	
}

void RenderPipeline::Initialize()
{
}

void RenderPipeline::UpdateTime()
{
}
#define STB_IMAGE_IMPLEMENTATION
//void RenderPipeline::Draw()
//{
//	for (auto& object : objects)
//	{
//		object->GetShader()->use();
//		glBindVertexArray(object->GetVAO());
//
//		glm::mat4 model = glm::mat4(1.0f);
//		model = glm::translate(model, object->GetPosition());
//		model = glm::rotate(model, glm::radians(object->GetRotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
//		model = glm::rotate(model, glm::radians(object->GetRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
//		model = glm::rotate(model, glm::radians(object->GetRotation().z), glm::vec3(0.0f, 0.0f, 1.0f));
//		model = glm::scale(model, object->GetScale());
//
//		object->GetShader()->setMat4("model", model);
//
//		unsigned int textureID;
//		if (Textures.TextureIDs.size() > 0)
//			textureID = Textures.TextureIDs[0];
//		else
//			textureID = 0;
//
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, textureID);
//		glDrawArrays(GL_TRIANGLES, 0, 36);
//	}
//}

void RenderPipeline::DrawDebugAxes(float length = 1.0f)
{
	GLfloat vertices[] = {
		// Positions          // Colors
		-0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, // Red
		 0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f  // Green
	};

	// Create and bind a VAO
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void RenderPipeline::DrawTriangle(unsigned int inVAO, int GL_METHOD, Shader* inShader)
{
	

	inShader->use();
	glBindVertexArray(inVAO);
	glDrawArrays(GL_METHOD, 0, 3);
}

void RenderPipeline::AddObject(RObject* object)
{
	RenderPipeline::objects.push_back(object);
}

void RenderPipeline::RenderDrawFrame(unsigned int inVAO, int GL_METHOD, Shader* inShader)
{
	
	inShader->use();
	glBindVertexArray(inVAO);
	glDrawArrays(GL_METHOD, 0, 3);
}

void RenderPipeline::ObjectDrawTest()
{
	std::cout << "Starting object draw test" << std::endl;
}

void RenderPipeline::DrawBlankFrame(Shader* inShader)
{
	

	inShader->use();
}

void RenderPipeline::RenderBackground()
{
	glClearColor(.2f, .3f, .3f, .75f);
	
}

void RenderPipeline::BeginFrame()
{
	glfwPollEvents(); // Start Frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderPipeline::EndFrame(GLFWwindow* window)
{
	glfwSwapBuffers(window); // End Frame
}