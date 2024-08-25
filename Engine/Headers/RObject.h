#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../Headers/Shader.h"
#include <iostream>

class RObject
{
public:

	RObject(unsigned int vao, Shader* shader) : VAO(vao), shader(shader), position(0.0f), rotation(0.0f), scale(1.0f) {}
	void SetPosition(const glm::vec3& pos) { position = pos; }
	void SetRotation(const glm::vec3& rot) { rotation = rot; }
	void SetScale(const glm::vec3& scl) { scale = scl; }

	unsigned int GetVAO() const { return VAO; }
	Shader* GetShader() const { return shader; }
	glm::vec3 GetPosition() const { return position; }
	glm::vec3 GetRotation() const { return rotation; }
	glm::vec3 GetScale() const { return scale; }

	static unsigned int QuickGenerateCubeVAO();

private:
	unsigned int VAO;
	Shader* shader;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};