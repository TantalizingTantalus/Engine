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



class Model : public Entity
{
public:
	// public globals
	
	bool IsLight = false;
	std::string fullFilePath;
	RENDERTARGETS RenderMode = RENDERTARGETS::NORMAL;
	std::string modelName;

	// Utility
	Model(std::string path) 
	{
		fullFilePath = path;
		loadModel(fullFilePath);
		std::string newPath = path;
		
		std::filesystem::path filePath = newPath;
		std::string fileName = filePath.filename().string();
		newPath = fileName;
		SetModelFileName(newPath);
		Name = fileName;
	}

	

	Model()
	{
		this->modelName = "null_model";
	}

	void Draw();

	// Setters and Getters

	void SetModelName(std::string name) { this->modelName = name; }
	std::string GetModelName() const { return this->modelName; }

	void SetModelFileName(std::string name) { this->fileName = name; }
	std::string GetModelFileName() const { return this->fileName; }

	void SetShader(Shader& inShader) { this->shader = &inShader; }
	GLuint GetShaderID() { return shader->ID; }
	Shader& GetShader() { return *shader; }

	Model Duplicate() const {
		Model copy = *this;  // Assuming copy constructor handles deep copying
		// Perform additional logic if necessary (e.g., reset certain fields)
		return copy;
	}

	bool& GetVisible() { return this->RenderModel; }
	void SetVisible(const bool inFlag) { this->RenderModel = inFlag; }

private:

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