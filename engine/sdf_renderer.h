#pragma once
#include "shader.h"
#include "render_mesh.h"
#include <array>
#include <GL/glew.h>
#include <matrix.hpp>

namespace Engine
{
	struct SdfRenderTarget
	{
		GLuint framebuffer;
		GLuint distanceTexture;
		struct
		{
			int x;
			int y;
		} textureSize;
	};

	class SdfRenderer
	{
	private:
		std::array<SdfRenderTarget, 4> renderTargets;
		Shader conePassShader;
		Shader colorPassShader;
		RenderMesh screenQuad;

	public:
		SdfRenderer();
		~SdfRenderer();
		
		void Init(int windowWidth, int windowHeight);
		void Reload(const std::string& sdfCode);
		void Draw(class Camera& camera, const glm::mat4& cameraTransform, float time);
	};
}