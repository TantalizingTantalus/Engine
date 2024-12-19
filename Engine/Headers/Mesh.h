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
	int entityID;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

enum RENDERTARGETS {
	LIT,
	LINES,
	NORMAL,
	RENDER_COUNT
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	RENDERTARGETS RenderMode = RENDERTARGETS::LIT;
	

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	
	void Draw(Shader& shader);

private:
	unsigned int VAO, VBO, EBO;
	unsigned int instanceVBO;
	
	void SetupMesh();

};