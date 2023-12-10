#include "transform.h"
#include "game_object.h"

Transform::Transform(GameObject* _p_owner) :
	Component(_p_owner),
	matrix(1.f),
	position(0.f),
	rotation(glm::identity<glm::quat>()),
	scale(1.f)
{}

Transform::~Transform()
{}

bool Transform::CanBeCastedTo(ComponentId id)
{
	return id.id == GetTypeId<Transform>().id || Component::CanBeCastedTo(id);
}