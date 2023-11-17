#pragma once
#include <vec3.hpp>
#include <gtc/quaternion.hpp>

namespace Engine
{
	struct HitResult
	{
		glm::vec3 point;
		glm::vec3 normal;
		float distance;

		HitResult();
	};

	class Collider
	{
	public:
		glm::vec3 position;
		glm::quat rotation;

		Collider();

		virtual bool IntersectsSDF(float(*sdf)(const glm::vec3&), HitResult& outHitResult) = 0;
		float (*ShapeSDF)(Collider*, const glm::vec3&);
	};

	class SphereCollider final : public Collider
	{
	public:
		float radius;

		SphereCollider();

		virtual bool IntersectsSDF(float(*sdf)(const glm::vec3&), HitResult& outHitReslut) override;
	};

	glm::vec3 CalcNormal(Collider* p_collider, const glm::vec3& p);
	glm::vec3 CalcNormal(float(*sdf)(const glm::vec3&), const glm::vec3& p);

	inline HitResult::HitResult() :
		point(0.f),
		normal(0.f),
		distance(0.f)
	{}

	inline Collider::Collider() :
		position(0.f),
		rotation(glm::identity<glm::quat>()),
		ShapeSDF(nullptr)
	{}
}