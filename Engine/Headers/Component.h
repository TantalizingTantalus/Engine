#pragma once
#include "imgui-master/backends/imgui_impl_glfw.h"
#include "imgui-master/backends/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include "imgui-master/imgui.h"
#include "imgui-master/imgui_internal.h"
#include <iostream>
#include <vector>

class Entity;

class Component
{
public:
	Entity* parentEntity;
	virtual void ShowImGuiPanel() = 0;
	void SetParent(Entity* parent)
	{
		parentEntity = parent;
	}
};

