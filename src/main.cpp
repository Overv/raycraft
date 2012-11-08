// Raycraft internals
#include <rc/world.hpp>
#include <rc/renderer.hpp>

#include <GL/glfw.h>

#include <ctime>

// Configuration
const int WIDTH = 1280;
const int HEIGHT = 720;

int main()
{
	// Initialize glfw
	glfwInit();

	// Open window
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindow(WIDTH, HEIGHT, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle("raycraft");
	glfwSwapInterval(0);

	// Center window on screen
	GLFWvidmode videoMode;
	glfwGetDesktopMode(&videoMode);
	glfwSetWindowPos(videoMode.Width / 2 - WIDTH / 2, videoMode.Height / 2 - HEIGHT / 2);

	// Create simple world
	rc::world world(20, 20, 20);
	world.createFlatWorld(5);

	// Shiny gold wall
	for (int x = 13; x <= 18; x++)
		for (int z = 5; z <= 8; z++)
			world.set(x, 5, z, rc::material::GOLD);

	// Arch being reflected
	world.set(17, 8, 5, rc::material::GRASS);
	world.set(17, 8, 6, rc::material::GRASS);
	world.set(16, 8, 6, rc::material::GRASS);
	world.set(15, 8, 6, rc::material::GRASS);
	world.set(15, 8, 5, rc::material::GRASS);

	// Cages
	world.set(3, 17, 5, rc::material::CAGE);
	world.set(3, 17, 6, rc::material::CAGE);
	world.set(4, 17, 5, rc::material::CAGE);
	world.set(4, 17, 6, rc::material::CAGE);

	// Tree
	world.set(8, 12, 5, rc::material::WOOD);
	world.set(8, 12, 6, rc::material::WOOD);
	for (int x = 7; x <= 9; x++)
		for (int y = 11; y <= 13; y++)
			for (int z = 7; z <= 9; z++)
				world.set(x, y, z, rc::material::LEAF);
	world.set(8, 12, 7, rc::material::WOOD);

	// Create renderer
	rc::renderer renderer;
	renderer.setWorld(world);

	// Main loop
	char titleBuf[128];
	int frames = 0, curTime = time(nullptr);
	int lastMousePos[2];
	int lastMouseLeft = 0, lastMouseRight = 0;
	float lastYaw = 0.785, yaw = 0.785;

	while(glfwGetWindowParam(GLFW_OPENED))
	{
		// Handle input
		int x, y;
		glfwGetMousePos(&x, &y);

		// Allow user to rotate view and destroy blocks
		if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			if (lastMouseLeft == 0) {
				lastMouseLeft = GLFW_PRESS;
				glfwGetMousePos(&lastMousePos[0], &lastMousePos[1]);
			}

			yaw = lastYaw - (float)(x - lastMousePos[0]) / 70.0f;
		} else if (lastMouseLeft == GLFW_PRESS) {
			if (abs(lastMousePos[0] - x) < 2 && abs(lastMousePos[1] - y) < 2) {
				glm::vec3 pos, normal;
				renderer.pick(x, y, pos, normal);
				world.set(pos.x, pos.y, pos.z, rc::material::EMPTY);
			}

			lastMouseLeft = 0;
			lastYaw = yaw;
		}

		// Allow user to create blocks
		if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			if (lastMouseRight == 0) {
				lastMouseRight = GLFW_PRESS;
				glfwGetMousePos(&lastMousePos[0], &lastMousePos[1]);
			}
		} else if (lastMouseRight == GLFW_PRESS) {
			if (abs(lastMousePos[0] - x) < 2 && abs(lastMousePos[1] - y) < 2) {
				glm::vec3 pos, normal;
				renderer.pick(x, y, pos, normal);
				world.set(pos.x + normal.x, pos.y + normal.y, pos.z + normal.z, rc::material::STONE);
			}

			lastMouseRight = 0;
		}

		// Update view
		renderer.setCameraTarget(glm::vec3(cos(yaw) * 17.0f + 10.0f, sin(yaw) * 17.0f + 10.0f, 12.0f), glm::vec3(10.0f, 10.0f, 0.0f), 70.0f, (float)WIDTH / (float)HEIGHT);

		// Draw frame
		renderer.drawFrame();

		// Present
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