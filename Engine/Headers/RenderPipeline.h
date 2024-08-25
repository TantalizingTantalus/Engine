#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gl/GL.h>

#include "../Headers/Shader.h"
#include "../Headers/RObject.h"
#include "../Headers/TextureLoader.h"

#include <vector>
#include <iostream>
class RenderPipeline
{
public:
	RenderPipeline();
	void Initialize();
	void Draw();
	void DrawTriangle(unsigned int, int, Shader*);
	void ObjectDrawTest();
	void RenderDrawFrame(unsigned int, int, Shader*);
	void DrawBlankFrame(Shader*);
	void RenderBackground();
	static void BeginFrame();
	static void EndFrame(GLFWwindow*);
	static void DrawDebugAxes(float );

	void AddObject(RObject* object);
private:
	std::vector<RObject*> objects;
	TextureLoader Textures;
};