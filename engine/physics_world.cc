#include "physics_world.h"
#include <map>
#include <gtx/matrix_cross_product.hpp>

namespace Engine
{
	PhysicsWorld::PhysicsWorld() :
		worldSDF(nullptr),
		worldPhysicsMaterial({0.f, 0.f}),
		gravity(0.f)
	{}

	void PhysicsWorld::FindAabbIntersections()
	{
		aabbIntersections.clear();

		constexpr size_t sweepAxis = 0;
		constexpr size_t secondaryAxis1 = 1;
		constexpr size_t secondaryAxis2 = 2;

		std::multimap<float, EventPoint> events;
		std::unordered_map<size_t, EventPoint*> activeSet;// key = event point's pair-id

		// sort "events" (where an aabb has a min or max value on the axis)
		// inserting into map based on distance on axis grants sorted order
		// insertion should be log(N)
		size_t pairId = 0;
		for (PhysicsObject& object : objects)
		{
			for (size_t i = 0; i < object.colliders.size(); i++)
			{
				events.insert({ object.colliders[i]->worldAABB.min[sweepAxis], {EventPoint::Type::E_Start, pairId, &object, i} });
				events.insert({ object.colliders[i]->worldAABB.max[sweepAxis], {EventPoint::Type::E_End, pairId, &object, i} });
				pairId++;
			}
		}

		// move plane from min to max and find overlaps in the axis direction
		for (auto eventItr = events.begin(); eventItr != events.end(); eventItr++)
		{
			size_t nextPairId = eventItr->second.pairId;
			EventPoint* p_nextEvent = &eventItr->second;

			if (p_nextEvent->type == EventPoint::Type::E_End)
				activeSet.erase(nextPairId);
			else
			{
				for (auto& active : activeSet)
				{
					EventPoint* p_activeEvent = active.second;

					if (p_activeEvent->p_object == p_nextEvent->p_object)
						continue;// ignore self

					// perform aabb vs aabb check along the remaining axis
					const AABB& aabb1 = p_activeEvent->p_object->colliders[p_activeEvent->colliderIndex]->worldAABB;
					const AABB& aabb2 = p_nextEvent->p_object->colliders[p_nextEvent->colliderIndex]->worldAABB;
					if (aabb1.min[secondaryAxis1] <= aabb2.max[secondaryAxis1] && aabb1.max[secondaryAxis1] >= aabb2.min[secondaryAxis1] &&
						aabb1.min[secondaryAxis2] <= aabb2.max[secondaryAxis2] && aabb1.max[secondaryAxis2] >= aabb2.min[secondaryAxis2])
					{
						aabbIntersections.push_back({
							p_activeEvent->p_object,
							p_activeEvent->colliderIndex,
							p_nextEvent->p_object,
							p_nextEvent->colliderIndex
						});
					}
				}

				activeSet.insert({ p_nextEvent->pairId, p_nextEvent });
			}
		}
	}

	glm::mat3 MakeContactMatrix(const glm::vec3& hitNormal)
	{
		glm::vec3 tangent1, tangent2;

		if (glm::abs(hitNormal.x) > glm::abs(hitNormal.y))
		{
			float scale = 1.f / glm::sqrt(hitNormal.z * hitNormal.z + hitNormal.x * hitNormal.x);

			tangent1.x = hitNormal.z * scale;
			tangent1.y = 0.f;
			tangent1.z = -hitNormal.x * scale;

			tangent2.x = hitNormal.y * tangent1.x;
			tangent2.y = hitNormal.z * tangent1.x - hitNormal.x * tangent1.z;
			tangent2.z = -hitNormal.y * tangent1.x;
		}
		else
		{
			float scale = 1.f / glm::sqrt(hitNormal.z * hitNormal.z + hitNormal.y * hitNormal.y);

			tangent1.x = 0.f;
			tangent1.y = -hitNormal.z * scale;
			tangent1.z = hitNormal.y * scale;

			tangent2.x = hitNormal.y * tangent1.z - hitNormal.z * tangent1.y;
			tangent2.y = -hitNormal.x * tangent1.z;
			tangent2.z = hitNormal.x * tangent1.y;
		}

		return glm::mat3(
			hitNormal,
			tangent1,
			tangent2
		);
	}

	glm::vec3 PhysicsWorld::CalculateImpulseResponse(
		const glm::vec3& hitPoint,
		const glm::vec3& hitNormal,
		const Rigidbody* p_firstRb,
		const Rigidbody* p_optionalSecondRb,
		const PhysicsMaterial& firstPhysicsMat,
		const PhysicsMaterial& secondPhysicsMat
	)
	{
		float restitution = firstPhysicsMat.restitution * secondPhysicsMat.restitution;
		float friction = 0.5f * (firstPhysicsMat.friction + secondPhysicsMat.friction);

		glm::mat3 contactToWorld(MakeContactMatrix(hitNormal));
		glm::mat3 worldToContact(glm::transpose(contactToWorld));
		glm::vec3 relativePoint[2]{
			hitPoint - p_firstRb->centerOfMass,
			glm::vec3(0.f)
		};
		glm::vec3 angularVel[2]{
			p_firstRb->angularVelocity,
			glm::vec3(0.f)
		};
		glm::vec3 pointVel[2]{
			(p_firstRb->linearVelocity + glm::cross(angularVel[0], relativePoint[0])),
			glm::vec3(0.f)
		};
		glm::vec3 relativeVel(pointVel[0]);

		if (p_optionalSecondRb != nullptr)
		{
			relativePoint[1] = hitPoint - p_optionalSecondRb->centerOfMass;
			angularVel[1] = p_optionalSecondRb->angularVelocity;
			pointVel[1] = p_optionalSecondRb->linearVelocity + glm::cross(angularVel[1], relativePoint[1]);
			relativeVel = pointVel[1] - pointVel[0];
		}

		// transform relative velocity to contact space
		glm::vec3 contactVel = worldToContact * relativeVel;
		float desiredNormalVelocity = -contactVel.x * (1.f + restitution);
		float totalInverseMass = p_firstRb->inverseMass;

		// build matrix that converts a unit of impulse to a unit of torque
		glm::mat3 impulseToTorque(glm::matrixCross3(relativePoint[0]));

		// build a matrix that converts an impulse in contact-space to a change in velocity in world-space
		glm::mat3 deltaVelWorld(impulseToTorque);
		deltaVelWorld *= p_firstRb->worldInverseInertiaTensor;
		deltaVelWorld *= impulseToTorque;
		deltaVelWorld *= -1.f;

		if (p_optionalSecondRb != nullptr)
		{
			// do the same procedure for the second rigidbody and add result to deltaVelWorld
			impulseToTorque = glm::matrixCross3(relativePoint[1]);
			glm::mat3 deltaVelWorld2(impulseToTorque);
			deltaVelWorld2 *= p_optionalSecondRb->worldInverseInertiaTensor;
			deltaVelWorld2 *= impulseToTorque;
			deltaVelWorld2 *= -1.f;

			deltaVelWorld += deltaVelWorld2;
			totalInverseMass += p_optionalSecondRb->inverseMass;
		}

		// create a contact-space version of the deltaVelWorld
		glm::mat3 deltaVelContact(worldToContact);
		deltaVelContact *= deltaVelWorld;
		deltaVelContact *= contactToWorld;

		// the change in linear velocity for a unit impulse is the sum of the inverse masses
		deltaVelContact[0][0] += totalInverseMass;
		deltaVelContact[1][1] += totalInverseMass;
		deltaVelContact[2][2] += totalInverseMass;

		// inverting gives matrix that converts from unit velocity to impulse in contact-space
		glm::mat3 impulseMatrix(glm::inverse(deltaVelContact));

		// contact-space point-velocity we want to subtract by applying impulse
		// we want to make the point stand still in the contact plane and bounce back in the normal direction
		glm::vec3 subtractVel(
			desiredNormalVelocity,
			-contactVel.y,
			-contactVel.z
		);

		// calculate the contact-space impulse needed to produce the velocity subtraction
		glm::vec3 impulse(impulseMatrix * subtractVel);

		// calculate the length of the impulse in the contact plane
		float planarImpulse = glm::sqrt(
			impulse.y * impulse.y +
			impulse.z * impulse.z
		);

		// check if the planar component of the impulse exceeds the static friction limit
		// if it does, we change the impulse to simulate dynamic friction
		if (planarImpulse > impulse.x * friction)
		{
			impulse.y /= planarImpulse;
			impulse.z /= planarImpulse;
		
			impulse.x = deltaVelContact[0][0] +
				deltaVelContact[0][1] * friction * impulse.y +
				deltaVelContact[0][2] * friction * impulse.z;
			impulse.x = desiredNormalVelocity / impulse.x;
			impulse.y *= friction * impulse.x;
			impulse.z *= friction * impulse.x;
		}

		// convert back to world space
		impulse = contactToWorld * impulse;

		return impulse;
	}

	void PhysicsWorld::Init(float(*_worldSDF)(const glm::vec3&), const PhysicsMaterial& _worldPhysicsMaterial)
	{
		worldSDF = _worldSDF;
		worldPhysicsMaterial = _worldPhysicsMaterial;
	}

	void PhysicsWorld::AddObject(const std::vector<Collider*>& colliders, Rigidbody* p_rigidbody, const PhysicsMaterial& physicsMaterial)
	{
		objects.push_back({ colliders, p_rigidbody, physicsMaterial });
	}

	PhysicsObject* PhysicsWorld::RaycastObjects(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, HitResult& outHitResult)
	{
		PhysicsObject* p_closestObj = nullptr;

		for (PhysicsObject& object : objects)
		{
			for (Collider* p_collider : object.colliders)
			{
				HitResult hit;
				if (p_collider->IntersectsRay(origin, direction, hit) && hit.distance < maxDistance)
				{
					maxDistance = hit.distance;
					outHitResult = hit;
					p_closestObj = &object;
				}
			}
		}

		return p_closestObj;
	}

	void PhysicsWorld::Start()
	{
		for (PhysicsObject& object : objects)
		{
			glm::mat4 rbWorldMatrix = glm::mat4_cast(object.p_rigidbody->rotation);
			rbWorldMatrix[3] = glm::vec4(object.p_rigidbody->centerOfMass, 1.f);

			for (Collider* p_collider : object.colliders)
			{
				p_collider->worldMatrix = rbWorldMatrix * p_collider->localMatrix;
				p_collider->UpdateWorldAABB();
			}
		}
	}

	void PhysicsWorld::Update(float deltaTime)
	{
		for (PhysicsObject& object : objects)
			object.p_rigidbody->ApplyGravity(gravity, deltaTime);

		FindAabbIntersections();

		collisions.clear();

		// object vs object
		for (AabbIntersection& intersection : aabbIntersections)
		{
			Collider* p_firstCollider = intersection.p_firstObject->colliders[intersection.firstColliderIndex];
			Collider* p_secondCollider = intersection.p_secondObject->colliders[intersection.secondColliderIndex];

			HitResult hit;
			if (p_firstCollider->IntersectsSDF(p_secondCollider->sdf, hit))
			{
				glm::vec3 impulse = CalculateImpulseResponse(
					hit.point,
					hit.normal,
					intersection.p_firstObject->p_rigidbody,
					intersection.p_secondObject->p_rigidbody,
					intersection.p_firstObject->physicsMaterial,
					intersection.p_secondObject->physicsMaterial
				);

				glm::vec3 overlap = hit.normal * (hit.distance * 0.5f);

				collisions.push_back({ intersection.p_firstObject, hit.point, -impulse, overlap });
				collisions.push_back({ intersection.p_secondObject, hit.point, impulse, -overlap });
			}
		}

		// object vs world
		for (PhysicsObject& object : objects)
		{
			for (Collider* p_collider : object.colliders)
			{
				HitResult hit;
				if (p_collider->IntersectsSDF(worldSDF, hit))
				{
					glm::vec3 impulse = CalculateImpulseResponse(
						hit.point,
						hit.normal,
						object.p_rigidbody,
						nullptr,
						object.physicsMaterial,
						worldPhysicsMaterial
					);

					glm::vec3 overlap = hit.normal * hit.distance;

					collisions.push_back({ &object, hit.point, impulse, overlap });
				}
			}
		}

		for (Collision& collision : collisions)
			collision.p_object->p_rigidbody->AddCollisionResponseTranslation(collision.overlap);

		for (Collision& collision : collisions)
			collision.p_object->p_rigidbody->AddImpulseAtPoint(collision.impulse, collision.hitPoint);

		for (PhysicsObject& object : objects)
		{
			object.p_rigidbody->Integrate(deltaTime);

			glm::mat4 rbWorldMatrix = glm::mat4_cast(object.p_rigidbody->rotation);
			rbWorldMatrix[3] = glm::vec4(object.p_rigidbody->centerOfMass, 1.f);

			for (Collider* p_collider : object.colliders)
			{
				p_collider->worldMatrix = rbWorldMatrix * p_collider->localMatrix;
				p_collider->UpdateWorldAABB();
			}
		}
	}
}