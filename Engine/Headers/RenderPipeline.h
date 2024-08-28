#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gl/GL.h>

#include "../Headers/Shader.h"

#include <vector>
#include <iostream>
class RenderPipeline
{
public:
	RenderPipeline();
	static void Initialize();
	void Draw();
	void DrawTriangle(unsigned int, int, Shader*);
	void ObjectDrawTest();
	void RenderDrawFrame(unsigned int, int, Shader*);
	void DrawBlankFrame(Shader*);
	static void RenderBackground();
	static void BeginFrame();
	static void EndFrame(GLFWwindow*);
	static void DrawDebugAxes(float );

	static void UpdateTime();
private:
};