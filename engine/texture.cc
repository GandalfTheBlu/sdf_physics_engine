#include "texture.h"
#include "debug.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Engine
{
	TextureCube::TextureCube() : 
		texture(0)
	{}

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &texture);
	}

	void TextureCube::Reload(const std::string texturePaths[6])
	{
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

        int width = 0;
        int height = 0;
        int channels = 0;

        for (size_t i = 0; i < 6; i++)
        {
            stbi_uc* data = stbi_load(texturePaths[i].c_str(), &width, &height, &channels, 0);
            if (data != nullptr)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else
            {
                Affirm(false, "failed to load cube map texture '", texturePaths[i], "'");
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

    void TextureCube::Bind(GLuint binding) const
    {
        glActiveTexture(binding);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    }

    void TextureCube::Unbind(GLuint binding) const
    {
        glActiveTexture(binding);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
}