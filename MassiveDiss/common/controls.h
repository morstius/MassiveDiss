#ifndef CONTROLS_H
#define CONTROLS_H

void update();

void updateMVP(GLFWwindow*, GLFWmonitor*, int, int);
void handleMouse(GLFWwindow*, int, int);
void handleKeyboard(GLFWwindow*, GLFWmonitor*, int, int);

glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

glm::vec3 getPosition();

void setPosition(glm::vec3);

float getVertical();
float getHorizontal();

void setVertical(float);
void setHorizontal(float);

bool getVisCheck();
bool getOccCheck();

bool getMode();
bool reverseOcclusionCulling();

#endif