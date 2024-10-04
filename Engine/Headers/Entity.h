#pragma once
#include <iostream>
#include <string>
#include "../Headers/Transform.h"

class Entity 
{
public:
	Entity()
	{
		
	}

	Entity* parent;
	std::vector<Entity*> children;

	Transform transform;
};