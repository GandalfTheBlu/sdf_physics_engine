#include "sdf.h"
#include <glm.hpp>

namespace Engine
{
	glm::vec3 CalcNormal(const SDF& sdf, const glm::vec3& p)
	{
		constexpr float h = 0.0001;
		constexpr float x = 1.f;
		constexpr float y = -1.f;

		return glm::normalize(
			glm::vec3(x, y, y) * sdf(p + glm::vec3(x, y, y) * h) +
			glm::vec3(y, y, x) * sdf(p + glm::vec3(y, y, x) * h) +
			glm::vec3(y, x, y) * sdf(p + glm::vec3(y, x, y) * h) +
			glm::vec3(x, x, x) * sdf(p + glm::vec3(x, x, x) * h));
	}
}