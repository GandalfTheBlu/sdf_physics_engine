#pragma once
#include <glm.hpp>
#include <gtx/quaternion.hpp>

namespace Engine
{
	class Rigidbody final
	{
	public:
		glm::vec3 centerOfMass;
		glm::vec3 linearVelocity;
		glm::vec3 accumilatedForce;
		float inverseMass;
		float linearDamping;

		glm::quat rotation;
		glm::quat angularVelocity;
		glm::vec3 accumilatedTorque;
		glm::mat3 localInverseInertiaTensor;
		float angularDamping;

		glm::mat3 worldInverseInertiaTensor;
		glm::vec3 accumilatedResponseTranslation;

		Rigidbody();

		void SetMass(float mass);
		void SetInertiaTensor(const glm::mat3& inertiaTensor);

		void AddForce(const glm::vec3& force);
		void AddForceAtPoint(const glm::vec3& force, const glm::vec3& point);
		void AddGravityForce(const glm::vec3& gravity);
		void AddImpulse(const glm::vec3& impulse);
		void AddImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& point);
		void AddCollisionResponseTranslation(const glm::vec3& responseTranslation);

		void Integrate(float deltaTime);
	};
}