#include "../Headers/Entity.h"

void Entity::AddChild(Entity* entToAdd)
{
	children.emplace_back(entToAdd);
	children.back()->parent = this;
	
}
