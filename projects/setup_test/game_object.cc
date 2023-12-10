#include "game_object.h"
#include "component.h"
#include <algorithm>

GameObject::GameObject()
{}

GameObject::~GameObject()
{
	for (Component* p_comp : ownedComponents)
		delete p_comp;
}

void GameObject::DestroyPendingComponents()
{
	std::sort(componentsToDestroy.begin(), componentsToDestroy.end());
	for (int i = (int)componentsToDestroy.size() - 1; i >= 0; i--)
	{
		size_t index = componentsToDestroy[i];
		delete ownedComponents[index];
		ownedComponents.erase(ownedComponents.begin() + index);
	}

	componentsToDestroy.clear();
}

void GameObject::Start()
{}

void GameObject::Update(float deltaTime)
{
	for (Component* p_comp : ownedComponents)
		p_comp->EarlyUpdate(deltaTime);

	DestroyPendingComponents();

	for (Component* p_comp : ownedComponents)
		p_comp->LateUpdate(deltaTime);

	DestroyPendingComponents();
}


void GameObject::RemoveComponent(Component* p_component)
{
	for (size_t i = 0; i < ownedComponents.size(); i++)
	{
		if (ownedComponents[i] == p_component)
		{
			componentsToDestroy.push_back(i);
			return;
		}
	}
}