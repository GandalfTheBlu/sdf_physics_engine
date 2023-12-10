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


void Transform::LateUpdate(float deltaTime)
{
	UpdateMatrix();
}

void Transform::UpdateMatrix()
{
	glm::mat3 m = glm::mat3(0.f);
	m[0][0] = scale.x;
	m[1][1] = scale.y;
	m[2][2] = scale.z;

	m *= glm::mat3_cast(rotation);

	matrix = m;
	matrix[3] = glm::vec4(position, 1.f);
}