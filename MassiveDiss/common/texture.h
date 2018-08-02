#ifndef TEXTURE_H
#define TEXTURE_H

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(const char * imagepath);

GLuint loadStbText(const char * imagepath);

#endif