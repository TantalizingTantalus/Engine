#pragma once
#include "imgui-master/backends/imgui_impl_glfw.h"
#include "imgui-master/backends/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include "imgui-master/imgui.h"
#include "imgui-master/imgui_internal.h"
#include <iostream>
#include <vector>

class Component
{
public:
	virtual void ShowImGuiPanel() = 0;
	
};

class ComponentLibrary
{
public:
	std::vector<std::shared_ptr<Component>> c_Library;

	void AddComponent(std::shared_ptr<Component> myComp)
	{
		c_Library.push_back(myComp);
	}

	template <typename T>
	T& GetComponent() {
		for (auto& component : c_Library) {
			if (auto castedComponent = std::dynamic_pointer_cast<T>(component)) {
				return castedComponent;
			}
		}
		throw std::runtime_error("Component not found!");
	}
};