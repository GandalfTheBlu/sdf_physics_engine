#include "rigidbody.h"

namespace Engine
{
	Rigidbody::Rigidbody() :
		centerOfMass(glm::vec3(0.f)),
		linearVelocity(glm::vec3(0.f)),
		accumilatedForce(glm::vec3(0.f)),
		inverseMass(1.f),
		linearDamping(0.2f),
		rotation(glm::identity<glm::quat>()),
		angularVelocity(glm::identity<glm::quat>()),
		accumilatedTorque(glm::vec3(0.f)),
		localInverseInertiaTensor(glm::identity<glm::mat3>()),
		angularDamping(0.2f),
		worldInverseInertiaTensor(glm::identity<glm::mat3>()),
		accumilatedResponseTranslation(glm::vec3(0.f))
	{}

	void Rigidbody::AddForce(const glm::vec3& force)
	{
		accumilatedForce += force;
	}

	void Rigidbody::AddForceAtPoint(const glm::vec3& force, const glm::vec3& point)
	{
		accumilatedForce += force;
		accumilatedTorque += glm::cross(point - centerOfMass, force);
	}

	void Rigidbody::AddGravityForce(const glm::vec3& gravity)
	{
		if (inverseMass != 0.f)
			accumilatedForce += gravity / inverseMass;
	}

	void Rigidbody::AddImpulse(const glm::vec3& impulse)
	{
		linearVelocity += impulse * inverseMass;
	}

	void Rigidbody::AddImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& point)
	{
		linearVelocity += impulse * inverseMass;
		angularVelocity += glm::quat(0.f, worldInverseInertiaTensor * glm::cross(point - centerOfMass, impulse));
	}

	void Rigidbody::AddCollisionResponseTranslation(const glm::vec3& responseTranslation)
	{
		accumilatedResponseTranslation += responseTranslation;
	}

	void Rigidbody::SetMass(float mass)
	{
		inverseMass = 1.f / mass;
	}

	void Rigidbody::SetInertiaTensor(const glm::mat3& inertiaTensor)
	{
		localInverseInertiaTensor = glm::inverse(inertiaTensor);
	}

	void Rigidbody::Integrate(float fixedDeltaTime)
	{
		// update linear velocity based on force and mass (linear acceleration)
		// remove component in response translation direction
		linearVelocity += accumilatedForce * (fixedDeltaTime * inverseMass);

		if (accumilatedResponseTranslation != glm::vec3(0.f))
		{
			glm::vec3 translationNormal = glm::normalize(accumilatedResponseTranslation);
			float velocityNormalComponent = glm::dot(translationNormal, linearVelocity);

			// if velocity and translation are opposite in direction
			if (velocityNormalComponent < 0.f)
				linearVelocity -= translationNormal * velocityNormalComponent;
		}

		// apply linear damping
		linearVelocity *= glm::pow(1.f - glm::clamp(linearDamping, 0.f, 0.999f), fixedDeltaTime);

		// update center of mass based on linear velocity and response translation
		centerOfMass += linearVelocity * fixedDeltaTime + accumilatedResponseTranslation;


		// convert inertia tensor to world space
		glm::mat3 rotationMatrix(glm::mat3_cast(rotation));
		worldInverseInertiaTensor = rotationMatrix * localInverseInertiaTensor * glm::transpose(rotationMatrix);

		// calculate angular acceleration and represent it as a quaternion with 0 scalar value
		glm::quat angularAcceleration(0.f, worldInverseInertiaTensor * accumilatedTorque);

		// update angular velocity based on angular acceleration
		angularVelocity += angularAcceleration * fixedDeltaTime;

		// apply angular damping
		angularVelocity *= glm::pow(1.f - glm::clamp(angularDamping, 0.f, 0.999f), fixedDeltaTime);

		// update rotation based on angular velocity
		rotation += (0.5f * fixedDeltaTime) * angularVelocity * rotation;
		rotation = glm::normalize(rotation);


		// reset accumilators
		accumilatedForce = glm::vec3(0.f);
		accumilatedTorque = glm::vec3(0.f);
		accumilatedResponseTranslation = glm::vec3(0.f);
	}
}