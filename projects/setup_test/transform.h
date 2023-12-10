#pragma once
#include "component.h"
#include <matrix.hpp>
#include <vec3.hpp>
#include <gtx/quaternion.hpp>

class Transform : public Component
{
protected:
	virtual bool CanBeCastedTo(ComponentId id) override;

public:
	glm::mat4 matrix;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	Transform(GameObject* _p_owner);
	virtual ~Transform();

	virtual void LateUpdate(float deltaTime) override;

	void UpdateMatrix();
};