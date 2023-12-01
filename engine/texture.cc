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
        GLuint newTexture = 0;
        glGenTextures(1, &newTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, newTexture);

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
                stbi_image_free(data);
                Affirm(false, "failed to load cube map texture '", texturePaths[i], "'");
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        if (texture != 0)
            glDeleteTextures(1, &texture);

        texture = newTexture;
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


    Texture2D::Texture2D() : 
        texture(0)
    {}

    Texture2D::~Texture2D()
    {
        glDeleteTextures(1, &texture);
    }

    void Texture2D::Reload(const std::string& texturePath)
    {
        stbi_set_flip_vertically_on_load(true);

        int colorChannels = 0;
        int width = 0;
        int height = 0;
        unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &colorChannels, 0);
        Affirm(data != nullptr, "failed to load texture '", texturePath, "'");

        GLenum glFormat = GL_RGB;
        switch (colorChannels)
        {
        case 1:
            glFormat = GL_R;
            break;
        case 2:
            glFormat = GL_RG;
            break;
        case 3:
            glFormat = GL_RGB;
            break;
        case 4:
            glFormat = GL_RGBA;
            break;
        }

        if (texture != 0)
            glDeleteTextures(1, &texture);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)glFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
    }

    void Texture2D::Bind(GLuint binding) const
    {
        glActiveTexture(binding);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void Texture2D::Unbind(GLuint binding) const
    {
        glActiveTexture(binding);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}