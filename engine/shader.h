#pragma once
#include <GL/glew.h>
#include <string>
#include <map>

namespace Engine
{
	class Shader final
	{
	private:
		GLuint program;
		std::map<std::string, GLint> nameToLocation;

		void AddLocationIfNeeded(const std::string& name);

		void Deinit();

	public:
		Shader();
		~Shader();
		
		void Reload(const std::string& vertexFilePath, const std::string& fragmentFilePath);

		void Use();
		void StopUsing();

		void SetFloat(const std::string& name, GLfloat value);
		void SetInt(const std::string& name, GLint value);
		void SetVec2(const std::string& name, const GLfloat* valuePtr, GLsizei count = 1);
		void SetVec3(const std::string& name, const GLfloat* valuePtr, GLsizei count = 1);
		void SetVec4(const std::string& name, const GLfloat* valuePtr, GLsizei count = 1);
		void SetMat3(const std::string& name, const GLfloat* valuePtr, GLsizei count = 1);
		void SetMat4(const std::string& name, const GLfloat* valuePtr, GLsizei count = 1);
	};
}