#pragma once
#include "imgui/imgui-master/backends/imgui_impl_glfw.h"
#include "imgui/imgui-master/backends/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include "spdlog/spdlog.h"
#include "imgui/imgui-master/imgui.h"
#include "imgui/imgui-master/imgui_internal.h"
#include "SystemIcons.h"
#include <iostream>
#include <vector>

class Entity;

class Component
{
public:
	Entity* parent;

	virtual void ShowImGuiPanel() = 0;
	void SetParent(Entity* parentEntity)
	{
		parent = parentEntity;
		
	}
};

