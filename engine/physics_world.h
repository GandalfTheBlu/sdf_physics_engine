#pragma once
#include "collider.h"
#include "rigidbody.h"
#include <vector>

namespace Engine
{
	struct PhysicsMaterial
	{
		float restitution;
		float friction;
	};

	struct PhysicsObject
	{
		Collider* p_collider;
		Rigidbody* p_rigidbody;
		PhysicsMaterial physicsMaterial;
	};

	struct Collision
	{
		PhysicsObject* p_object;
		glm::vec3 hitPoint;
		glm::vec3 impulse;
		glm::vec3 overlap;
	};

	struct AabbIntersection
	{
		PhysicsObject* p_firstObject;
		PhysicsObject* p_secondObject;
	};

	struct EventPoint
	{
		enum class Type : char
		{
			E_Start,
			E_End
		};

		Type type;
		size_t pairId;
		PhysicsObject* p_object;
	};

	class PhysicsWorld final
	{
	private:
		std::vector<PhysicsObject> objects;
		SDF worldSDF;
		PhysicsMaterial worldPhysicsMaterial;

		std::vector<AabbIntersection> aabbIntersections;

		void PhysicsWorld::FindAabbIntersections();

		glm::vec3 PhysicsWorld::CalculateImpulseResponse(
			const glm::vec3& hitPoint,
			const glm::vec3& hitNormal,
			const Rigidbody* p_firstRb,
			const Rigidbody* p_optionalSecondRb,
			const PhysicsMaterial& firstPhysicsMat,
			const PhysicsMaterial& secondPhysicsMat);

	public:
		std::vector<Collision> collisions;
		glm::vec3 gravity;

		PhysicsWorld();

		void Init(float(*_worldSDF)(const glm::vec3&), const PhysicsMaterial& _worldPhysicsMaterial);
		void AddObject(Collider* p_collider, Rigidbody* p_rigidbody, const PhysicsMaterial& physicsMaterial);

		PhysicsObject* RaycastObjects(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, HitResult& outHitResult, Collider* p_ignore = nullptr);
		bool RaycastWorld(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, HitResult& outHitResult);

		void Start();
		void Update(float deltaTime);
	};
}