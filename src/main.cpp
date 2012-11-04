// Raycraft internals
#include <rc/world.hpp>
#include <rc/renderer.hpp>

#include <GL/glfw.h>

#include <ctime>

// Configuration
const int WIDTH = 320;
const int HEIGHT = 240;

int main()
{
	// Initialize glfw
	glfwInit();

	// Open window
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindow(WIDTH, HEIGHT, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle("raycraft");

	// Center window on screen
	GLFWvidmode videoMode;
	glfwGetDesktopMode(&videoMode);
	glfwSetWindowPos(videoMode.Width / 2 - WIDTH / 2, videoMode.Height / 2 - HEIGHT / 2);

	// Create world
	rc::world world(4, 4, 4);
	world.createFlatWorld(2);

	// Create renderer
	rc::renderer renderer;
	renderer.setCameraTarget(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), 45.0f, (float)WIDTH / (float)HEIGHT);

	// Main loop
	int frames = 0, curTime = time(nullptr);
	char titleBuf[128];

	while(glfwGetWindowParam(GLFW_OPENED))
	{
		// Draw frame
		renderer.drawFrame();
		glfwSwapBuffers();

		// Measure and display fps
		frames++;
		if (curTime != time(nullptr))
		{
			sprintf(titleBuf, "raycraft (%d fps)", frames);
			glfwSetWindowTitle(titleBuf);

			frames = 0;
			curTime = time(nullptr);
		}
	}

	// Clean up
	glfwTerminate();

	return 0;
}