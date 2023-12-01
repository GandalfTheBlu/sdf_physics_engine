#pragma once
#include <GL/glew.h>
#include <string>

namespace Engine
{
	class TextureCube
	{
	private:
		GLuint texture;

	public:
		TextureCube();
		~TextureCube();

		void Reload(const std::string texturePaths[6]);

		void Bind(GLuint binding) const;
		void Unbind(GLuint binding) const;
	};

	class Texture2D
	{
	private:
		GLuint texture;

	public:
		Texture2D();
		~Texture2D();

		void Reload(const std::string& texturePath);

		void Bind(GLuint binding) const;
		void Unbind(GLuint bindine) const;
	};
}