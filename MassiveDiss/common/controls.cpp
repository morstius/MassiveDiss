// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.h"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

// Initial position
glm::vec3 position = glm::vec3( 0, 0, 1 ); 
// Initial horizontal angle
float horizontalAngle = 2.0f;
// Initial vertical angle
float verticalAngle = 0.0f;
// Field of View
float fieldOfView = 45.0f;

bool visCheck = true;
bool tPressed = false;

bool occlusionCullingEnabled = false;
bool cPressed = false;

bool mode = true;
bool mPressed = false;

bool showBoundingVolume = false;
bool vPressed = false;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.0005f;

glm::vec3 getPosition() 
{
	return position;
}
glm::vec3 getDirection() 
{
	return glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
}

float getHorizontal()
{
	return horizontalAngle;
}

float getVertical()
{
	return verticalAngle;
}

bool getVisCheck()
{
	return visCheck;
}

bool getOccCheck()
{
	return occlusionCullingEnabled;
}

bool getMode()
{
	return mode;
}

bool showBounding()
{
	return showBoundingVolume;
}

void computeMatricesFromInputs(GLFWwindow* window, GLFWmonitor* monitor, float width, float height)
{
	// handle mouse
	handleMouse(window, width, height);

	// handle keyboard
	handleKeyboard(window, monitor);

	// Projection matrix : 45° Field of View,, display range : 0.1 unit <-> 100 units
	float aspect = 4.0f / 3.0f;			// 4:3 ratio
	float near = 0.1f;					// near
	float far = 100.0f;					// far

	ProjectionMatrix = glm::perspective(glm::radians(fieldOfView), aspect, near, far);
}

void handleMouse(GLFWwindow* window, float width, float height)
{
	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(width / 2 - xpos);
	verticalAngle += mouseSpeed * float(height / 2 - ypos);
}

void handleKeyboard(GLFWwindow* window, GLFWmonitor* monitor)
{
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double now = glfwGetTime();
	float dt = float(now - lastTime);

	// need to convert from Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// setting the horizondal direction
	glm::vec3 horizontalDir = direction;
	// don't want to fly or leave my plane without using q/e
	horizontalDir.y = 0.0f;

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS 
		|| glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += horizontalDir * dt * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS 
		|| glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= horizontalDir * dt * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS 
		|| glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * dt * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS 
		|| glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * dt * speed;
	}
	// Go up
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		position += vec3(0, 1.0f, 0) * dt * speed;
	}
	// Go down
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		position -= vec3(0, 1.0f, 0) * dt * speed;
	}

	//////////////////////////////////////////////
	////////////// TOGGLES ///////////////////////
	//////////////////////////////////////////////

	// Go fullscreen
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) 
	{
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		glViewport(0, 0, mode->width, mode->height);
	}
	// Go windowed
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) 
	{
		glfwSetWindowMonitor(window, NULL, 50, 50, 1024, 768, 0);
		glViewport(0, 0, 1024, 768);
	}

	// Toggle Frustum Culling
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) 
	{
		tPressed = true;
	}
	if (tPressed && glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) 
	{
		tPressed = false;
		visCheck = !visCheck;
	}
	// Toggle Occlusion Culling
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) 
	{
		cPressed = true;
	}
	if (cPressed && glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) 
	{
		cPressed = false;
		occlusionCullingEnabled = !occlusionCullingEnabled;
	}
	// Toggle Drawing Mode
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) 
	{
		mPressed = true;
	}
	if (mPressed && glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) 
	{
		mPressed = false;
		mode = !mode;
	}
	// Toggle Show Bounding Volume
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) 
	{
		vPressed = true;
	}
	if (vPressed && glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) 
	{
		vPressed = false;
		showBoundingVolume = !showBoundingVolume;
	}


	// Close window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}


	// Camera matrix, updating view matrix here since I calculated the direction and the up vector here
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);


	// For the next frame, the "last time" will be "now"
	// to have a nice delta time
	lastTime = now;
}