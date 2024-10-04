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
	RENDERTARGETS RenderMode = RENDERTARGETS::NORMAL;

	// Utility
	Model(std::string path) 
	{
		loadModel(path);
		DecomposeModelMatrix();
		std::string newPath = path;
		
		std::filesystem::path filePath = newPath;
		std::string fileName = filePath.filename().string();
		newPath = fileName;
		SetModelFileName(newPath);
	}

	Model()
	{
		this->modelName = "null_model";
	}

	void Draw();

	void DecomposeModelMatrix();
	void SetModelMatrix(glm::mat4 modelMat) { this->transform.modelMatrix = modelMat; UpdateModelMatrix(); }
	void UpdateModelMatrix();

	// Setters and Getters

	void SetRotation(float angle, const glm::vec3& axis) { transform.eulerRot = transform.eulerRot; transform.eulerRot = angle * axis; }
	void SetPosition(const glm::vec3& pos);
	

	void SetScale(const glm::vec3& scl) { transform.scale = scl; }

	void SetModelName(std::string name) { this->modelName = name; }
	std::string GetModelName() const { return this->modelName; }

	void SetModelFileName(std::string name) { this->fileName = name; }
	std::string GetModelFileName() const { return this->fileName; }

	glm::mat4& GetModelMatrix() { SafeDecompose(); return this->transform.modelMatrix; }

	void SafeDecompose() { try { DecomposeModelMatrix(); } catch (std::exception e) { spdlog::error(e.what()); } }

	glm::vec3& GetRotation() { SafeDecompose(); return transform.eulerRot; }
	glm::vec3& GetPosition() { SafeDecompose(); return transform.pos; }
	glm::vec3& GetScale() { SafeDecompose(); return transform.scale; }

	void SetShader(Shader& inShader) { this->shader = &inShader; }
	GLuint GetShaderID() { return shader->ID; }
	Shader& GetShader() { return *shader; }


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
	std::string modelName;
	std::string fileName;
};