#include "sdf_renderer.h"
#include "camera.h"
#include "default_meshes.h"
#include "debug.h"

namespace Engine
{
	SdfRenderer::SdfRenderer()
	{}

	void SdfRenderer::Init(int windowWidth, int windowHeight)
	{
		GenerateUnitQuad(screenQuad);

		const float sizeScales[4]
		{
			1.f,
			2.f / 3.f,
			1.f / 2.f,
			1.f / 4.f
		};

		for (size_t i = 0; i < renderTargets.size(); i++)
		{
			float fw = windowWidth * sizeScales[i];
			float fh = windowHeight * sizeScales[i];
			Affirm(fw >= 2.f && fh >= 2.f, "too small sdf render target texture size, use fewer render passes or larger window");

			int w = static_cast<int>(fw);
			int h = static_cast<int>(fh);

			GLuint framebuffer = 0;
			GLuint distanceTexture = 0;
			
			glGenFramebuffers(1, &framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

			glGenTextures(1, &distanceTexture);
			glBindTexture(GL_TEXTURE_2D, distanceTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, distanceTexture, 0);

			GLuint attachments[]{ GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, attachments);

			Affirm(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
				"failed to create sdf render target framebuffer");

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);

			renderTargets[renderTargets.size() - 1 - i] = SdfRenderTarget{ framebuffer, distanceTexture, {w, h} };
		}
	}

	void SdfRenderer::Reload(const std::string& sdfCode)
	{
		conePassShader.Reload("assets/shaders/sdf_vert.glsl", "assets/shaders/sdf_cone_pass_frag.glsl", {"__SDF__", sdfCode});
		colorPassShader.Reload("assets/shaders/sdf_vert.glsl", "assets/shaders/sdf_color_pass_frag.glsl", {"__SDF__", sdfCode});
	}

	SdfRenderer::~SdfRenderer()
	{
		for (size_t i = 0; i < renderTargets.size(); i++)
		{
			glDeleteFramebuffers(1, &renderTargets[i].framebuffer);
			glDeleteTextures(1, &renderTargets[i].distanceTexture);
		}
	}

	void SdfRenderer::Draw(Camera& camera, const glm::mat4& cameraTransform, float time)
	{
		glm::mat4 VP = camera.CalcP() * camera.CalcV(cameraTransform);
		glm::mat4 invVP = glm::inverse(VP);

		const int iterationCount[4]
		{
			12,
			12,
			12,
			64
		};

		screenQuad.Bind();
		conePassShader.Use();
		conePassShader.SetMat4("u_invVP", &invVP[0][0]);
		conePassShader.SetInt("u_isFirstPass", 1);
		conePassShader.SetFloat("u_time", time);

		for (size_t i=0; i<renderTargets.size(); i++)
		{
			float textureDiagonal = glm::length(glm::vec2(renderTargets[i].textureSize.x, renderTargets[i].textureSize.y));
			conePassShader.SetFloat("u_textureDiagonal", textureDiagonal);
			conePassShader.SetInt("u_iterationCount", iterationCount[i]);

			glBindFramebuffer(GL_FRAMEBUFFER, renderTargets[i].framebuffer);
			glViewport(0, 0, renderTargets[i].textureSize.x, renderTargets[i].textureSize.y);
			glClear(GL_COLOR_BUFFER_BIT);	

			if (i > 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, renderTargets[i - 1].distanceTexture);
			}

			screenQuad.Draw(0);

			if (i == 0)
				conePassShader.SetInt("u_isFirstPass", 0);
		}
		conePassShader.StopUsing();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		colorPassShader.Use();
		colorPassShader.SetMat4("u_VP", &VP[0][0]);
		colorPassShader.SetMat4("u_invVP", &invVP[0][0]);
		colorPassShader.SetFloat("u_time", time);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderTargets.back().distanceTexture);

		screenQuad.Draw(0);

		colorPassShader.StopUsing();
		glBindTexture(GL_TEXTURE_2D, 0);
		screenQuad.Unbind();
	}
}