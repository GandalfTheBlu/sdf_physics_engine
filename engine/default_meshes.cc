#include "default_meshes.h"
#include <glm.hpp>

namespace Engine
{
	void GenerateUnitQuad(RenderMesh& outMesh)
	{
		GLubyte indices[]
		{
			0, 2, 1,
			0, 3, 2
		};

		GLfloat positions[]
		{
			-1.f, -1.f,
			-1.f, 1.f,
			1.f, 1.f,
			1.f, -1.f
		};

		DataBuffer indexBuffer;
		indexBuffer.bufferStart = indices;
		indexBuffer.byteSize = sizeof(indices);

		IndexAttribute indexAttrib;
		indexAttrib.mode = GL_TRIANGLES;
		indexAttrib.offset = 0;
		indexAttrib.count = 6;
		indexAttrib.type = GL_UNSIGNED_BYTE;

		DataBuffer posBuffer;
		posBuffer.bufferStart = (GLubyte*)&positions;
		posBuffer.byteSize = sizeof(positions);

		VertexAttribute posAttrib;
		posAttrib.location = 0;
		posAttrib.components = 2;
		posAttrib.stride = sizeof(GLfloat) * 2;
		posAttrib.offset = 0;
		posAttrib.type = GL_FLOAT;

		outMesh.Reload(indexBuffer, { indexAttrib }, { posBuffer }, { posAttrib });
	}

	void GenerateUnitLine(RenderMesh& outMesh)
	{
		GLubyte indices[]
		{
			0, 1
		};

		GLfloat positions[]
		{
			0.f, 0.f, 0.f,
			0.f, 0.f, 1.f
		};

		DataBuffer indexBuffer;
		indexBuffer.bufferStart = indices;
		indexBuffer.byteSize = sizeof(indices);

		IndexAttribute indexAttrib;
		indexAttrib.mode = GL_LINES;
		indexAttrib.offset = 0;
		indexAttrib.count = 2;
		indexAttrib.type = GL_UNSIGNED_BYTE;

		DataBuffer posBuffer;
		posBuffer.bufferStart = (GLubyte*)&positions;
		posBuffer.byteSize = sizeof(positions);

		VertexAttribute posAttrib;
		posAttrib.location = 0;
		posAttrib.components = 3;
		posAttrib.stride = sizeof(GLfloat) * 3;
		posAttrib.offset = 0;
		posAttrib.type = GL_FLOAT;

		outMesh.Reload(indexBuffer, { indexAttrib }, { posBuffer }, { posAttrib });
	}

	void GenerateUnitCube(RenderMesh& outMesh)
	{
		GLubyte indices[]{
			// left
			0,3,1,
			3,2,1,
			// right
			4,7,5,
			7,6,5,
			// back
			8,11,9,
			11,10,9,
			// forward
			12,15,13,
			15,14,13,
			// down
			16,19,17,
			19,18,17,
			// up
			20,23,21,
			23,22,21
		};

		GLfloat positions[]{
			// left (0,1,2,3)
			-1.f, -1.f, 1.f,
			-1.f, 1.f, 1.f,
			-1.f, 1.f, -1.f,
			-1.f, -1.f, -1.f,
			// right (4,5,6,7)
			1.f, -1.f, -1.f,
			1.f, 1.f, -1.f,
			1.f, 1.f, 1.f,
			1.f, -1.f, 1.f,
			// back (8,9,10,11)
			-1.f, -1.f, -1.f,
			-1.f, 1.f, -1.f,
			1.f, 1.f, -1.f,
			1.f, -1.f, -1.f,
			// forward (12,13,14,15)
			1.f, -1.f, 1.f,
			1.f, 1.f, 1.f,
			-1.f, 1.f, 1.f,
			-1.f, -1.f, 1.f,
			// down (16,17,18,19)
			-1.f, -1.f, 1.f,
			-1.f, -1.f, -1.f,
			1.f, -1.f, -1.f,
			1.f, -1.f, 1.f,
			// up (20,21,22,23)
			-1.f, 1.f, -1.f,
			-1.f, 1.f, 1.f,
			1.f, 1.f, 1.f,
			1.f, 1.f, -1.f
		};

		DataBuffer indexBuffer;
		indexBuffer.bufferStart = indices;
		indexBuffer.byteSize = sizeof(indices);

		IndexAttribute indexAttrib;
		indexAttrib.mode = GL_TRIANGLES;
		indexAttrib.offset = 0;
		indexAttrib.count = 36;
		indexAttrib.type = GL_UNSIGNED_BYTE;

		DataBuffer posBuffer;
		posBuffer.bufferStart = (GLubyte*)&positions;
		posBuffer.byteSize = sizeof(positions);

		VertexAttribute posAttrib;
		posAttrib.location = 0;
		posAttrib.components = 3;
		posAttrib.stride = sizeof(GLfloat) * 3;
		posAttrib.offset = 0;
		posAttrib.type = GL_FLOAT;

		outMesh.Reload(indexBuffer, { indexAttrib }, { posBuffer }, { posAttrib });
	}

	void GenerateUnitSphere(RenderMesh& outMesh)
	{
		const int resX = 10;
		const int resY = 10;
		const int vertexCount = 6 * (resX + 1) * (resY + 1);
		const int indexCount = 36 * resX * resY;
		const int positionCount = vertexCount * 3;

		std::vector<GLuint> indices;
		std::vector<GLfloat> positions;

		indices.reserve(indexCount);
		positions.reserve(positionCount);

		// face order: left, right, back, forward, down, up
		glm::vec3 faceCorners[6]{
			glm::vec3(-1.f, -1.f, 1.f),
			glm::vec3(1.f, -1.f, -1.f),
			glm::vec3(-1.f, -1.f, -1.f),
			glm::vec3(1.f, -1.f, 1.f),
			glm::vec3(-1.f, -1.f, 1.f),
			glm::vec3(-1.f, 1.f, -1.f)
		};

		// right and up vector for each face
		glm::vec3 faceAxis[12]{
			glm::vec3(0.f, 0.f, -1.f),
			glm::vec3(0.f, 1.f, 0.f),

			glm::vec3(0.f, 0.f, 1.f),
			glm::vec3(0.f, 1.f, 0.f),

			glm::vec3(1.f, 0.f, 0.f),
			glm::vec3(0.f, 1.f, 0.f),

			glm::vec3(-1.f, 0.f, 0.f),
			glm::vec3(0.f, 1.f, 0.f),

			glm::vec3(1.f, 0.f, 0.f),
			glm::vec3(0.f, 0.f, -1.f),

			glm::vec3(1.f, 0.f, 0.f),
			glm::vec3(0.f, 0.f, 1.f)
		};

		// construct faces
		for (int n = 0; n < 6; n++)
		{
			// create grid
			int faceStartIndex = n * (resX + 1) * (resY + 1);
			for (int i = 0; i < resX + 1; i++)
			{
				for (int j = 0; j < resY + 1; j++)
				{
					GLfloat u = (GLfloat)i / resX;
					GLfloat v = (GLfloat)j / resY;
					glm::vec3 gridPoint = faceCorners[n] + faceAxis[2 * n] * (2.f * u) + faceAxis[2 * n + 1] * (2.f * v);
					glm::vec3 vertex = glm::normalize(gridPoint);

					positions.push_back(vertex.x);
					positions.push_back(vertex.y);
					positions.push_back(vertex.z);

					// create indicis on the fly (not if on right- or top edge)
					if (i >= resX || j >= resY)
						continue;

					GLuint i00 = faceStartIndex + i * (resY + 1) + j;
					GLuint i01 = faceStartIndex + i * (resY + 1) + j + 1;
					GLuint i11 = faceStartIndex + (i + 1) * (resY + 1) + j + 1;
					GLuint i10 = faceStartIndex + (i + 1) * (resY + 1) + j;

					indices.push_back(i10);
					indices.push_back(i01);
					indices.push_back(i00);

					indices.push_back(i11);
					indices.push_back(i01);
					indices.push_back(i10);
				}
			}
		}

		DataBuffer indexBuffer;
		indexBuffer.bufferStart = (GLubyte*)indices.data();
		indexBuffer.byteSize = sizeof(GLuint) * indices.size();

		IndexAttribute indexAttrib;
		indexAttrib.mode = GL_TRIANGLES;
		indexAttrib.offset = 0;
		indexAttrib.count = indices.size();
		indexAttrib.type = GL_UNSIGNED_INT;

		DataBuffer posBuffer;
		posBuffer.bufferStart = (GLubyte*)positions.data();
		posBuffer.byteSize = sizeof(GLfloat) * positions.size();

		VertexAttribute posAttrib;
		posAttrib.location = 0;
		posAttrib.components = 3;
		posAttrib.stride = sizeof(GLfloat) * 3;
		posAttrib.offset = 0;
		posAttrib.type = GL_FLOAT;

		outMesh.Reload(indexBuffer, { indexAttrib }, { posBuffer }, { posAttrib });
	}
}