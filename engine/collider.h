#pragma once
#include "sdf.h"
#include "hit_result.h"
#include <matrix.hpp>

namespace Engine
{
	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;

		AABB();
	};

	class Collider
	{
	public:
		Collider();

		AABB worldAABB;
		glm::mat4 localMatrix;
		glm::mat4 worldMatrix;
		SDF sdf;

		virtual void UpdateWorldAABB() = 0;
		virtual bool IntersectsSDF(const SDF& otherSDF, HitResult& outHitResult) const = 0;
		virtual bool IntersectsRay(const glm::vec3& origin, const glm::vec3& direction, HitResult& outHitResult) const = 0;
	};

	class SphereCollider final : public Collider
	{
	public:
		float radius;

		SphereCollider();

		virtual void UpdateWorldAABB() override;
		virtual bool IntersectsSDF(const SDF& otherSDF, HitResult& outHitReslut) const override;
		virtual bool IntersectsRay(const glm::vec3& origin, const glm::vec3& direction, HitResult& outHitResult) const override;
	};

	class CapsuleCollider final : public Collider
	{
	public:
		float radius;
		float height;

		CapsuleCollider();

		virtual void UpdateWorldAABB() override;
		virtual bool IntersectsSDF(const SDF& otherSDF, HitResult& outHitReslut) const override;
		virtual bool IntersectsRay(const glm::vec3& origin, const glm::vec3& direction, HitResult& outHitResult) const override;
	};
}