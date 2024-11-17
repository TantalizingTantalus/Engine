#pragma once
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "spdlog/spdlog.h"
#include "ImGuizmo.h"
#include <vector>
#include <string>
#include <filesystem>
#include "../Headers/Entity.h"
#include "../Headers/Transform.h"
#include "../Headers/Shader.h"
#include "../Headers/Mesh.h"
#include "../Headers/stb_image.h"

// Keep in mind:
// Child classes are constructed AFTER parent classes
// I.E. parent classes are invoked first.
class Model : public Component
{
public:
	// public globals
	
	bool IsLight = false;
	std::string fullFilePath;
	RENDERTARGETS RenderMode = RENDERTARGETS::NORMAL;
	std::string modelName;
	Entity* parentEntity = nullptr;

	// Setup model name and path
	Model(std::string path) 
	{
		fullFilePath = path;
		loadModel(fullFilePath);
		std::string newPath = path;

		std::filesystem::path filePath = newPath;
		std::string fileName = filePath.filename().string();
		newPath = fileName;
		modelName = fileName.substr(0, fileName.length() - 4);
		for (auto& t : textures_loaded)
		{
			std::filesystem::path someFileName = t.path;
			std::string temp = someFileName.string();
			if (temp.find('/') == std::string::npos && temp.find('\\') == std::string::npos)
			{
				temp = fmt::format("../Engine/Models/{}", temp);
			}
			textureIcons.push_back((void*)(intptr_t)LoadFileIconID(temp.c_str()));
		}
	}

	void Populate(std::string path)
	{
		fullFilePath = path;
		loadModel(fullFilePath);
		std::string newPath = path;

		std::filesystem::path filePath = newPath;
		std::string fileName = filePath.filename().string();
		newPath = fileName;
		modelName = fileName.substr(0, fileName.length() - 4);
	}

	Model()
	{
		// model name, used for assimp
		this->modelName = "null_model";

		// Entity name, used for editor properties
		modelName = "null_model";
	}

	Entity& GetParent() { return *parentEntity; }

	void Draw();
	void ShowImGuiPanel() override
	{
		using namespace ImGui;
		if (CollapsingHeader("Model"))
		{
			char fileNameBuffer[50];
			char textureBuffer[100];
			strcpy_s(fileNameBuffer, fullFilePath.c_str());
			SeparatorText("Model Name");
			TextWrapped(modelName.c_str());
			
			Spacing();
			SeparatorText("File Path");
			InputText("##modelfiletextinput", fileNameBuffer, IM_ARRAYSIZE(fileNameBuffer));
			
			Spacing();
			SeparatorText("Textures");
			for (int i = 0; i < textures_loaded.size(); i++)
			{
				std::filesystem::path someFileName = textures_loaded[i].path;
				
				strcpy_s(textureBuffer, someFileName.filename().string().c_str());
				ImageButton("label", textureIcons[i], ImVec2(200, 200));
				TextWrapped(textureBuffer);
				Spacing();
			}
			Spacing();
		}
	}


	GLuint LoadFileIconID(const char* path)
	{
		GLuint textureID;
		int height, width, channels;

		unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
		if (data == nullptr) {
			spdlog::error(fmt::format("\nError loading icon at path:\n{}\nReason:\n{}", path, stbi_failure_reason()));
			int pwidth, pheight, pchannels;
			unsigned char* placeholderData = stbi_load("../Engine/Textures/Engine/placeholder.png", &pwidth, &pheight, &pchannels, 4);
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);

			// Texture params
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pwidth, pheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, placeholderData);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(placeholderData);
			return textureID;
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Texture params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
		return textureID;
	}

	// Setters and Getters


	void SetModelName(std::string name) { this->modelName = name; }
	std::string GetModelName() const { return this->modelName; }

	void SetModelFileName(std::string name) { this->fileName = name; }
	std::string GetModelFileName() const { return this->fileName; }

	void SetShader(Shader& inShader) { this->shader = &inShader; }
	GLuint GetShaderID() { return shader->ID; }
	Shader& GetShader() { return *shader; }
	
	
	bool& GetVisible() { return this->RenderModel; }
	void SetVisible(const bool inFlag) { this->RenderModel = inFlag; }

private:
	std::vector<ImTextureID > textureIcons;
	bool RenderModel = true;
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;
	bool DecomposeMatrix(const glm::mat4& matrix, glm::vec3& scale, glm::vec3& rotation, glm::vec3& translation);

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	
	Shader* shader;
	
	std::string fileName;
};