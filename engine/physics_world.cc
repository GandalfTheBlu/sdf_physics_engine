#include "physics_world.h"
#include <gtx/matrix_cross_product.hpp>

namespace Engine
{
	PhysicsWorld::PhysicsWorld() :
		worldSDF(nullptr),
		worldPhysicsMaterial({0.f, 0.f}),
		gravity(0.f)
	{}

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
			glm::vec3(p_firstRb->angularVelocity.x, p_firstRb->angularVelocity.y, p_firstRb->angularVelocity.z),
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
			angularVel[1] = glm::vec3(
				p_optionalSecondRb->angularVelocity.x,
				p_optionalSecondRb->angularVelocity.y,
				p_optionalSecondRb->angularVelocity.z
			);
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

	float PhysicsWorld::SdfObjects(const glm::vec3& point, PhysicsObject*& p_outClosest)
	{
		float closestDist = FLT_MAX;
		for (PhysicsObject& object : objects)
		{
			float dist = object.p_collider->ShapeSDF(object.p_collider, point);
			if (dist < closestDist)
			{
				closestDist = dist;
				p_outClosest = &object;
			}
		}

		return closestDist;
	}

	void PhysicsWorld::Init(float(*_worldSDF)(const glm::vec3&), const PhysicsMaterial& _worldPhysicsMaterial)
	{
		worldSDF = _worldSDF;
		worldPhysicsMaterial = _worldPhysicsMaterial;
	}

	void PhysicsWorld::AddObject(Collider* p_collider, Rigidbody* p_rigidbody, const PhysicsMaterial& physicsMaterial)
	{
		p_rigidbody->centerOfMass = p_collider->position;
		p_rigidbody->rotation = p_collider->rotation;
		objects.push_back({ p_collider, p_rigidbody, physicsMaterial });
	}

	PhysicsObject* PhysicsWorld::RaycastObjects(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, HitResult& outHitResult)
	{
		const float infinity = FLT_MAX;
		const float minRadius = 0.001f;

		PhysicsObject* p_closestObject = nullptr;

		float t = 0.f;
		float stepScale = 1.2;
		float prevRadius = 0.;
		float stepLength = 0.;

		for (int i = 0; i < 100; i++)
		{
			PhysicsObject* p_object = nullptr;
			float signedRadius = SdfObjects(origin + direction * t, p_object);
			float radius = abs(signedRadius);

			bool longStepFail = stepScale > 1.f && (radius + prevRadius) < stepLength;

			if (longStepFail)
			{
				stepLength -= stepScale * stepLength;
				stepScale = 1.f;
			}
			else
			{
				stepLength = signedRadius * stepScale;
			}

			prevRadius = radius;

			if (radius < minRadius)
			{
				p_closestObject = p_object;
				break;
			}

			if (!longStepFail && t > maxDistance)
				break;

			t += stepLength;
		}

		if (t > maxDistance)
			return nullptr;

		outHitResult.point = origin + direction * t;
		outHitResult.normal = CalcNormal(p_closestObject->p_collider, outHitResult.point);
		outHitResult.distance = t;

		return p_closestObject;
	}

	void PhysicsWorld::Update(float deltaTime)
	{
		collisions.clear();

		for (PhysicsObject& object : objects)
		{
			HitResult hit;
			if (object.p_collider->IntersectsSDF(worldSDF, hit))
			{
				glm::vec3 impulse = CalculateImpulseResponse(
					hit.point,
					hit.normal, 
					object.p_rigidbody,
					nullptr, 
					object.physicsMaterial, 
					worldPhysicsMaterial
				);

				collisions.push_back({ &object, hit.point, impulse, hit.normal * hit.distance });
			}
		}

		for (Collision& collision : collisions)
			collision.p_object->p_rigidbody->AddCollisionResponseTranslation(collision.overlap);

		for (Collision& collision : collisions)
			collision.p_object->p_rigidbody->AddImpulseAtPoint(collision.impulse, collision.hitPoint);

		for (PhysicsObject& object : objects)
		{
			object.p_rigidbody->AddGravityForce(gravity);
			object.p_rigidbody->Integrate(deltaTime);
			object.p_collider->position = object.p_rigidbody->centerOfMass;
			object.p_collider->rotation = object.p_rigidbody->rotation;
		}
	}
}