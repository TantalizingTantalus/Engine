#include "../Headers/Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	SetupMesh();
}

void Mesh::SetupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    // ids
    /*glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));*/

    // weights
   /* glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));*/

    
    glBindVertexArray(0);
}

void Mesh::Draw(Shader& shader)
{
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); 

        std::string name = textures[i].type;
        int uniformLocation = -1;

        if (name == "material.texture_diffuse")
            uniformLocation = glGetUniformLocation(shader.ID, "material.texture_diffuse");
        else if (name == "material.texture_normal")
            uniformLocation = glGetUniformLocation(shader.ID, "material.texture_normal");
        else if (name == "material.texture_specular")
            uniformLocation = glGetUniformLocation(shader.ID, "material.texture_specular");

        if (uniformLocation != -1)
        {
            glUniform1i(uniformLocation, i); 
            glBindTexture(GL_TEXTURE_2D, textures[i].id); 
        }
    }

    // draw mesh
    glBindVertexArray(VAO);

    // DrawElementsInstanced is important here for future instancing implementation
    switch (RenderMode)
    {
    case RENDERTARGETS::LIT:
        glUniform1i(glGetUniformLocation(shader.ID, "DEBUG_NORMAL"), false);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, 1);
        break;
    case RENDERTARGETS::LINES:
        glUniform1i(glGetUniformLocation(shader.ID, "DEBUG_NORMAL"), false);
        glDrawElementsInstanced(GL_LINES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, 1);
        break;
    case RENDERTARGETS::NORMAL:
        glUniform1i(glGetUniformLocation(shader.ID, "DEBUG_NORMAL"), true);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, 1);
        break;
    }
    

    glBindVertexArray(0);

    
    glActiveTexture(GL_TEXTURE0);
}