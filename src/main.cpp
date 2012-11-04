// GLFW
#include <GL/glfw.h>

// Raycraft internals
#include <rc/world.hpp>

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

	// Create world
	rc::world world(4, 4, 4);
	world.createFlatWorld(2);

	// Main loop
	while(glfwGetWindowParam(GLFW_OPENED))
	{
		glfwSwapBuffers();
	}

	// Clean up
	glfwTerminate();

	return 0;
}