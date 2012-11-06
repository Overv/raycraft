#include <rc/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

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
		initPickFramebuffer();

		// Load resources
		loadMaterialTexture();

		// Set defaults
		setSkyColor(glm::vec3(127.0f/255.0f, 204.0f/255.0f, 255.0f/255.0f));

		// Block data doesn't exist until world is assigned
		blockDataTexture = 0;
	}

	renderer::~renderer()
	{
		glDeleteTextures(1, &pickColorbuffer);
		glDeleteFramebuffers(1, &pickFramebuffer);

		glDeleteTextures(1, &materialsTexture);

		if (blockDataTexture > 0) {
			glDeleteTextures(1, &blockDataTexture);
		}

		glDeleteBuffers(1, &vertexBuffer);
		glDeleteVertexArrays(1, &vertexArray);
		glDeleteProgram(shaderProgram);
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
	}

	void renderer::setWorld(world& w)
	{
		// Clean up previous data
		if (blockDataTexture > 0) {
			glDeleteTextures(1, &blockDataTexture);
		}

		// Prepare data for shader
		std::vector<unsigned int> blockData(w.sizeX() * w.sizeY() * w.sizeZ());
		for (int i = 0; i < w.sizeX() * w.sizeY() * w.sizeZ(); i++)
			blockData[i] = w.get(i);

		// Upload data as texture
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &blockDataTexture);
		glBindTexture(GL_TEXTURE_3D, blockDataTexture);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, w.sizeX(), w.sizeY(), w.sizeZ(), 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &blockData[0]);

		// Enable sampler and pass data to shader
		glUniform1i(glGetUniformLocation(shaderProgram, "blockData"), 0);
		glUniform1ui(glGetUniformLocation(shaderProgram, "sx"), w.sizeX());
		glUniform1ui(glGetUniformLocation(shaderProgram, "sy"), w.sizeY());
		glUniform1ui(glGetUniformLocation(shaderProgram, "sz"), w.sizeZ());

		// Set iteration limit based on world size
		int maxIterations = w.sizeX() + w.sizeY() + w.sizeZ();
		glUniform1i(glGetUniformLocation(shaderProgram, "maxIterations"), maxIterations);

		// Register callback for world updates
		w.addBlockCallback([] (int x, int y, int z, material::material_t mat)
		{
			glActiveTexture(GL_TEXTURE0);
			glTexSubImage3D(GL_TEXTURE_3D, 0, x, y, z, 1, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &mat);
		});
	}

	void renderer::setSkyColor(const glm::vec3& color)
	{
		glUniform4f(glGetUniformLocation(shaderProgram, "skyColor"), color.x, color.y, color.z, 1.0f);
	}

	void renderer::setCameraDir(const glm::vec3& pos, const glm::vec3& dir, float fov, float aspect)
	{
		setCameraTarget(pos, pos + dir, fov, aspect);
	}

	void renderer::setCameraTarget(const glm::vec3& pos, const glm::vec3& target, float fov, float aspect)
	{
		// For some reason the projection changes when farZ is < 100.0, probably inaccuracies somewhere.
		// Luckily for us, it doesn't matter what farZ we use.
		glm::mat4 proj = glm::perspective(fov, aspect, 1.0f, 1000.f);
		glm::mat4 view = glm::lookAt(pos, target, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 invProjView = glm::inverse(proj * view);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "invProjView"), 1, GL_FALSE, glm::value_ptr(invProjView));
		glUniform3f(glGetUniformLocation(shaderProgram, "viewOrigin"), pos.x, pos.y, pos.z);
	}

	void renderer::drawFrame() const
	{
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void renderer::pick(int x, int y, glm::vec3& pos, glm::vec3& normal) const
	{
		// Draw scene in picking mode
		glBindFramebuffer(GL_FRAMEBUFFER, pickFramebuffer);
		glUniform1ui(glGetUniformLocation(shaderProgram, "pickMode"), GL_TRUE);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Read position and normal from selected pixel in window coordinates (y-flipped)
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		GLubyte pixel[4];
		glReadPixels(x, viewport[3] - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

		pos = glm::vec3(pixel[0], pixel[1], pixel[2]);

		switch (pixel[3]) {
			case 0: normal = glm::vec3(1, 0, 0); break;
			case 1: normal = glm::vec3(-1, 0, 0); break;
			case 2: normal = glm::vec3(0, 1, 0); break;
			case 3: normal = glm::vec3(0, -1, 0); break;
			case 4: normal = glm::vec3(0, 0, 1); break;
			case 5: normal = glm::vec3(0, 0, -1); break;
		}

		// Return to normal rendering mode
		glUniform1ui(glGetUniformLocation(shaderProgram, "pickMode"), GL_FALSE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
		std::ifstream file(path.c_str(), std::ios::ate);
		if (!file.is_open()) file.open(("bin/" + path).c_str(), std::ios::in | std::ios::ate);

		if (!file.is_open() || (int)file.tellg() == 0) {
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

	void renderer::initPickFramebuffer()
	{
		// Create frame buffer to hold picking result
		glGenFramebuffers(1, &pickFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, pickFramebuffer);

		// Create texture to hold color buffer
		glGenTextures(1, &pickColorbuffer);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pickColorbuffer);

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport[2], viewport[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Attach color buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickColorbuffer, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void renderer::loadMaterialTexture()
	{
		int w, h;
		unsigned char* pixels = SOIL_load_image("materials.png", &w, &h, 0, SOIL_LOAD_RGB);
		if (pixels == NULL) pixels = SOIL_load_image("bin/materials.png", &w, &h, 0, SOIL_LOAD_RGB);

		if (pixels == NULL) {
			printf("Couldn't load texture file 'materials.png'!\n");
			return;
		}

		glActiveTexture(GL_TEXTURE1);

		glGenTextures(1, &materialsTexture);
		glBindTexture(GL_TEXTURE_2D, materialsTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glUniform1i(glGetUniformLocation(shaderProgram, "materials"), 1);
		glUniform1f(glGetUniformLocation(shaderProgram, "materialCount"), 4);

		SOIL_free_image_data(pixels);
	}
}