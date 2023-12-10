#include "component.h"
#include "game_object.h"

Component::Component(GameObject* _p_owner) :
	p_owner(_p_owner)
{}

Component::~Component()
{}

Component::ComponentId Component::nextId = { 0 };

bool Component::CanBeCastedTo(ComponentId id)
{
	return id.id == GetTypeId<Component>().id;
}

GameObject& Component::GetOwner()
{
	return *p_owner;
}

void Component::EarlyUpdate(float deltaTime)
{}

void Component::LateUpdate(float deltaTime)
{}

void Component::Destroy()
{
	p_owner->RemoveComponent(this);
}