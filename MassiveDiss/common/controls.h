#ifndef CONTROLS_H
#define CONTROLS_H

void computeMVP(GLFWwindow*, GLFWmonitor*, int, int);
void handleMouse(GLFWwindow*, int, int);
void handleKeyboard(GLFWwindow*, GLFWmonitor*, int, int);

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