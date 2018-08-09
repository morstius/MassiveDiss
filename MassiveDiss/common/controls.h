#ifndef CONTROLS_H
#define CONTROLS_H

void updateMVP(GLFWwindow*, GLFWmonitor*, int, int);
void handleMouse(GLFWwindow*, int, int);
void handleKeyboard(GLFWwindow*, GLFWmonitor*, int, int);

glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

glm::vec3 getPosition();

float getVertical();
float getHorizontal();

bool getVisCheck();
bool getOccCheck();

bool getMode();
bool reverseOcclusionCulling();

#endif