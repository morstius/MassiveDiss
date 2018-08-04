#ifndef CONTROLS_H
#define CONTROLS_H

void computeMatricesFromInputs(GLFWwindow*, GLFWmonitor*, float, float);
void handleMouse(GLFWwindow*, float, float);
void handleKeyboard(GLFWwindow*, GLFWmonitor*);

glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

glm::vec3 getPosition();
glm::vec3 getDirection();

float getVertical();
float getHorizontal();

bool getVisCheck();
bool getOccCheck();

bool getMode();
bool showBounding();

#endif