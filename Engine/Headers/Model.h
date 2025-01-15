#pragma once
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "spdlog/spdlog.h"
#include "ImGuizmo.h"
#include <windows.h>
#include <commdlg.h>
#include <vector>
#include <string>
#include <filesystem>
#include "Entity.h"
#include "Model.h"
#include "Transform.h"
#include "Shader.h"
#include "Mesh.h"
#include "stb_image.h"
#include "SystemIcons.h"




// Keep in mind:
// Child classes are constructed AFTER parent classes
// I.E. parent classes are invoked first.
class Model : public Component
{
public:
	
	// public globals
	std::vector<Texture> textures_loaded;
	bool IsLight = false;
	std::string fullFilePath;
	RENDERTARGETS RenderMode = RENDERTARGETS::LIT;
	std::string modelName;
	Entity* parentEntity = nullptr;
	bool hasNormal = false;
	bool hasSpecular = false;
	int diffuseLocation;
	int normalLocation;
	int specularLocation;
	std::string directory;
	Shader* shader;

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
			char fileNameBuffer[100];
			TextWrapped(fmt::format("{}", hasNormal).c_str());
			strcpy_s(fileNameBuffer, fullFilePath.c_str());
			
			SeparatorText("Rendering");
			TextWrapped("Visible?");
			Checkbox("##ModelVisibilityCheckbox", &RenderModel);
			TextWrapped("Is Light?");
			Checkbox("##isLight", &IsLight);
			const char* renderModeNames[] = { "Lit", "Lines", "Normal Map" };
			Spacing();
			TextWrapped("Render Mode:");
			if (Combo("##RenderTargets", reinterpret_cast<int*>(&RenderMode), renderModeNames, RENDERTARGETS::RENDER_COUNT))
			{
				switch (RenderMode)
				{
				case RENDERTARGETS::LIT:
					RenderMode = RENDERTARGETS::LIT;
					
					break;
				case RENDERTARGETS::LINES:
					RenderMode = RENDERTARGETS::LINES;
					break;
				case RENDERTARGETS::NORMAL:
					RenderMode = RENDERTARGETS::NORMAL;
					
					break;
				}
			}
			Spacing();

			SeparatorText("Model Name");
			TextWrapped(modelName.c_str());
			
			Spacing();
			SeparatorText("File Path");
			InputText("##modelfiletextinput", fileNameBuffer, IM_ARRAYSIZE(fileNameBuffer));
			
			Spacing();
			
		}
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
	
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	
	std::string fileName;
};