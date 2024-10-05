#pragma once
#include <iostream>
#include <string>
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
		if (transform.isDirty()) {
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
			transform.computeModelMatrix(parent->transform.getModelMatrix());
		else
			transform.computeModelMatrix();

		for (auto&& child : children)
		{
			child->forceUpdateSelfAndChild();
		}
	}


	Entity* parent;
	std::vector<Entity*> children;
	glm::mat4 LocalModelMatrix;

	Transform transform;
};