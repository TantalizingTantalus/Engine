#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../Headers/Shader.h"
#include "../Headers/Mesh.h"
#include "../Headers/stb_image.h"
#include "glm/gtc/matrix_transform.hpp"
#include "spdlog/spdlog.h"
#include "ImGuizmo.h"



class Model
{
public:
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 position;

	Model(std::string path) : modelMatrix(1.0f)
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
		loadModel(path);
		DecomposeModelMatrix();
	}
	void Draw(Shader& shader);
	void SetRotation(float angle, const glm::vec3& axis) { modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis); }
	void SetPosition(const glm::vec3& pos) { modelMatrix = glm::translate(glm::mat4(1.0f), pos); }
	void SetScale(const glm::vec3& scl) { modelMatrix = glm::scale(modelMatrix, scl); }
	void SetModelName(std::string name) { this->modelName = name; }
	std::string GetModelName() const { return this->modelName; }
	glm::mat4 GetModelMatrix() { return this->modelMatrix; }
	void DecomposeModelMatrix()
	{
		DecomposeMatrix(modelMatrix, scale, rotation, position);
	}
	glm::vec3 GetRotation() { DecomposeModelMatrix(); return rotation; }
	glm::vec3 GetPosition() { DecomposeModelMatrix(); return position; }
	glm::vec3 GetScale() { DecomposeModelMatrix(); return scale; }

private:
	glm::mat4 modelMatrix;
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;
	bool DecomposeMatrix(const glm::mat4& matrix, glm::vec3& scale, glm::vec3& rotation, glm::vec3& translation)
	{
		// Extract the translation part
		translation = glm::vec3(matrix[3]);

		// Extract the scaling factors
		glm::vec3 row[3];
		row[0] = glm::vec3(matrix[0]);
		row[1] = glm::vec3(matrix[1]);
		row[2] = glm::vec3(matrix[2]);

		// Compute scale factors
		scale.x = glm::length(row[0]);
		scale.y = glm::length(row[1]);
		scale.z = glm::length(row[2]);

		// Normalize the rows
		row[0] = glm::normalize(row[0]);
		row[1] = glm::normalize(row[1]);
		row[2] = glm::normalize(row[2]);

		// Compute rotation matrix
		glm::mat3 rotationMatrix;
		rotationMatrix[0] = row[0];
		rotationMatrix[1] = row[1];
		rotationMatrix[2] = row[2];

		// Compute the rotation angles (assuming rotation is around the X, Y, and Z axes)
		glm::vec3 euler;
		euler.x = atan2(rotationMatrix[2][1], rotationMatrix[2][2]);
		euler.y = atan2(-rotationMatrix[2][0], sqrt(rotationMatrix[2][1] * rotationMatrix[2][1] + rotationMatrix[2][2] * rotationMatrix[2][2]));
		euler.z = atan2(rotationMatrix[1][0], rotationMatrix[0][0]);

		// Convert from radians to degrees
		rotation = glm::degrees(euler);

		return true;
	}

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	
	
	
	std::string modelName;
};