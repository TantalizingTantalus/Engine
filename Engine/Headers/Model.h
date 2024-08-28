#pragma once
#include <string>
#include <vector>
#include "../Headers/Shader.h"
#include "../Headers/Mesh.h"
#include "../Headers/stb_image.h"
#include "glm/gtc/matrix_transform.hpp"
#include "spdlog/spdlog.h"


class Model
{
public:
	Model(std::string path) : modelMatrix(1.0f)
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
		loadModel(path);
	}
	void Draw(Shader& shader);
	void SetRotation(float angle, const glm::vec3& axis) { modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis); }
	void SetPosition(const glm::vec3& pos) { modelMatrix = glm::translate(glm::mat4(1.0f), pos); }
	void SetScale(const glm::vec3& scl) { modelMatrix = glm::scale(modelMatrix, scl); }
	void SetModelName(std::string name) { this->modelName = name; }
	std::string GetModelName() const { return this->modelName; }

	glm::vec3 GetRotation() { return rotation; }
	glm::vec3 GetPosition() { return position; }
	glm::vec3 GetScale() { return scale; }

private:
	glm::mat4 modelMatrix;
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 position;

	std::string modelName;
};