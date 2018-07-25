#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void computeMatricesFromInputs(GLFWwindow*, GLFWmonitor*);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

glm::vec3 getPosition();
glm::vec3 getDirection();
float getFoV();
bool getVisCheck();
bool getOccCheck();

bool getMode();

#endif