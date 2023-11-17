#include "collider.h"
#include <glm.hpp>

namespace Engine
{
	glm::vec3 CalcNormal(Collider* p_collider, const glm::vec3& p)
	{
		const float h = 0.0001;
		const float x = 1.f;
		const float y = -1.f;
		return glm::normalize(
			glm::vec3(x, y, y) * p_collider->ShapeSDF(p_collider, p + glm::vec3(x, y, y) * h) +
			glm::vec3(y, y, x) * p_collider->ShapeSDF(p_collider, p + glm::vec3(y, y, x) * h) +
			glm::vec3(y, x, y) * p_collider->ShapeSDF(p_collider, p + glm::vec3(y, x, y) * h) +
			glm::vec3(x, x, x) * p_collider->ShapeSDF(p_collider, p + glm::vec3(x, x, x) * h));
	}

	glm::vec3 CalcNormal(float(*sdf)(const glm::vec3&), const glm::vec3& p)
	{
		const float h = 0.0001;
		const float x = 1.f;
		const float y = -1.f;
		return glm::normalize(
			glm::vec3(x, y, y) * sdf(p + glm::vec3(x, y, y) * h) +
			glm::vec3(y, y, x) * sdf(p + glm::vec3(y, y, x) * h) +
			glm::vec3(y, x, y) * sdf(p + glm::vec3(y, x, y) * h) +
			glm::vec3(x, x, x) * sdf(p + glm::vec3(x, x, x) * h));
	}

	SphereCollider::SphereCollider() :
		radius(1.f)
	{
		ShapeSDF = [](Collider* p_this, const glm::vec3& point)
		{
			SphereCollider* p_sphere = static_cast<SphereCollider*>(p_this);
			return glm::distance(point, p_sphere->position) - p_sphere->radius;
		};
	}

	bool SphereCollider::IntersectsSDF(float(*sdf)(const glm::vec3&), HitResult& outHitReslut)
	{
		float dist = sdf(position);

		if (dist > radius)
			return false;

		outHitReslut.normal = CalcNormal(sdf, position);
		outHitReslut.point = position - outHitReslut.normal * dist;
		outHitReslut.distance = radius - dist;// distance = overlap
		return true;
	}
}