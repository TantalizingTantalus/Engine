#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "../Headers/Transform.h"

class Entity 
{
public:
	std::string Name;
	Entity()
	{
		Name = "NewEntity";
		parent = nullptr;
	}


	void AddChild(Entity*);
	Entity* GetEntity() { return this; }

	void UpdateSelfAndChild()
	{
		if (transform->isDirty()) {
			forceUpdateSelfAndChild();
			return;
		}

		for (auto&& child : children)
		{
			child->UpdateSelfAndChild();
		}
	}

	void forceUpdateSelfAndChild()
	{
		if (parent)
			transform->computeModelMatrix(parent->transform->getModelMatrix());
		else
			transform->computeModelMatrix(transform->m_modelMatrix);

		for (auto&& child : children)
		{
			child->forceUpdateSelfAndChild();
		}
	}

	void AddComponent(std::shared_ptr<Component> component)
	{
		components.push_back(component);
	}

	void ShowComponents() {
		
		if (components.size() > 0)
		{
			for (const auto& component : components) {
				if(component != nullptr)
					component->ShowImGuiPanel();  
			}
		}
	}

	template <typename T>
	T& GetComponent() {
		for (auto& component : components) {
			if (auto castedComponent = std::dynamic_pointer_cast<T>(component)) {
				return *castedComponent;
			}
		}
		throw std::runtime_error("Component not found!");
	}

	Entity* parent;
	std::vector<Entity*> children;
	std::vector<std::shared_ptr<Component>> components;
	glm::mat4 LocalModelMatrix;

	std::shared_ptr<Transform> transform = std::make_shared<Transform>();
};