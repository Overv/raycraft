#ifndef RC_RENDERER_HPP
#define RC_RENDERER_HPP

#include <rc/world.hpp>
#include <glm/glm.hpp>
#include <GL3/gl3w.h>
#include <string>

namespace rc
{
	class renderer
	{
	public:
		renderer();
		~renderer();

		void setWorld(const world& w);

		void setCameraDir(const glm::vec3& pos, const glm::vec3& dir, float fov, float aspect);
		void setCameraTarget(const glm::vec3& pos, const glm::vec3& target, float fov, float aspect);

		void drawFrame() const;

	private:
		GLuint vertexShader, fragmentShader, shaderProgram;
		GLuint vertexArray, vertexBuffer;
		GLuint blockDataTexture;

		void initShaders();
		GLuint loadShader(const std::string& path, GLenum type);

		void initVertexData();
	};
}

#endif