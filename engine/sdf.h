#pragma once
#include <functional>
#include <vec3.hpp>

namespace Engine
{
	typedef std::function<float(const glm::vec3&)> SDF;

	glm::vec3 CalcNormal(const SDF& sdf, const glm::vec3& p);
}