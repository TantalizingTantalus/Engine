#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "../Headers/Shader.h"
#include "../Headers/Component.h"

#define MAX_BONE_INFLUENCE 4

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

enum RENDERTARGETS {
	NORMAL,
	LINES,
	RENDER_COUNT
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	RENDERTARGETS RenderMode = RENDERTARGETS::NORMAL;
	//std::vector<glm::mat4> instancedMats;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	
	void Draw(Shader& shader);

private:
	unsigned int VAO, VBO, EBO;
	unsigned int instanceVBO;
	
	void SetupMesh();

};