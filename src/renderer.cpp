#include <rc/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <vector>

namespace rc
{
	// Vertices for a screen filling quad
	float screenVertices[] = {
		-1.0f,  1.0f,
		 1.0f,  1.0f,
		 1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f,  1.0f,
	};

	renderer::renderer()
	{
		// Load OpenGL functions
		gl3wInit();

		// Initialize OpenGL
		initShaders();
		initVertexData();
	}

	renderer::~renderer()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteVertexArrays(1, &vertexArray);
		glDeleteProgram(shaderProgram);
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
	}

	void renderer::setCameraDir(const glm::vec3& pos, const glm::vec3& dir, float fov, float aspect)
	{
		setCameraTarget(pos, pos + dir, fov, aspect);
	}

	void renderer::setCameraTarget(const glm::vec3& pos, const glm::vec3& target, float fov, float aspect)
	{
		glm::mat4 proj = glm::perspective(fov, aspect, 1.0f, 10.f);
		glm::mat4 view = glm::lookAt(pos, target, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 invProjView = glm::inverse(proj * view);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "invProjView"), 1, GL_FALSE, glm::value_ptr(invProjView));
		glUniform3f(glGetUniformLocation(shaderProgram, "viewOrigin"), pos.x, pos.y, pos.z);
	}

	void renderer::drawFrame()
	{
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void renderer::initShaders()
	{
		vertexShader = loadShader("renderer.vert", GL_VERTEX_SHADER);
		fragmentShader = loadShader("renderer.frag", GL_FRAGMENT_SHADER);

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		glUseProgram(shaderProgram);
	}

	GLuint renderer::loadShader(const std::string& path, GLenum type)
	{
		// Locate shader file and read it
		std::ifstream file(path.c_str(), std::ios::in | std::ios::ate);
		if (!file.is_open()) file.open(("bin/" + path).c_str(), std::ios::in | std::ios::ate);

		if (!file.is_open() || file.tellg() == 0) {
			printf("Couldn't load shader file '%s'!\n", path.c_str());
			return 0;
		}

		int len = (int)file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<char> src(len);
		char* srcPtr = &src[0];
		file.read(&src[0], len);

		file.close();

		// Create and compile shader
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &srcPtr, &len);
		glCompileShader(shader);
		
		// Check if shader compiled successfully
		int status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			char buf[512];
			GLsizei length;
			glGetShaderInfoLog(shader, 512, &length, buf);
			printf("%s\n", buf);
		}

		return shader;
	}

	void renderer::initVertexData()
	{
		// Load vertex data
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);

		// Bind vertex data
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}
}