#include <vector>
#include <cstring>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.h"
#include "texture.h"

#include "text2D.h"

unsigned int Text2DTextureID;
unsigned int Text2DVertexBufferID;
unsigned int Text2DUVBufferID;
unsigned int Text2DShaderID;
unsigned int Text2DUniformID;

void initText2D(const char * texturePath)
{
	// initialize the buffers
	glGenBuffers(1, &Text2DVertexBufferID);
	glGenBuffers(1, &Text2DUVBufferID);

	// initialize the shaders
	Text2DShaderID = LoadShaders("shaders/text2D.vertexshader", "shaders/text2D.fragmentshader");

	// load in texture
	Text2DTextureID = loadStbText(texturePath);

	// get handle on the sampler
	Text2DUniformID = glGetUniformLocation( Text2DShaderID, "texSampler" );
}

void populateVector(std::vector<glm::vec2>& vec, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
	// add the abc triangle to the vector
	vec.push_back(a);
	vec.push_back(b);
	vec.push_back(c);
}

void populateVerticesPerLetter(std::vector<glm::vec2>& vertices, const int x, const int y, const unsigned int size, const int letter)
{
	// setting up the 4 corners to draw onto
	glm::vec2 up_left = glm::vec2(x + letter * size, y + size);
	glm::vec2 up_right = glm::vec2(x + letter * size + size, y + size);
	glm::vec2 down_right = glm::vec2(x + letter * size + size, y);
	glm::vec2 down_left = glm::vec2(x + letter * size, y);

	// upper triangle
	populateVector(vertices, up_left, down_left, up_right);

	// lower triangle
	populateVector(vertices, down_right, up_right, down_left);
}

void populateUVsPerLetter(std::vector<glm::vec2>& uvs, const char character)
{
	// for the uv coordinates we need to know how things are structured in the texture
	// the texture is set up in a grid like fashion so we can calculate the x and y from knowing
	// what character we want to pick out of the texture
	int numbLetters = 16;
	float scale = 1.0f / (float)numbLetters;
	float x = (character % numbLetters) * scale;
	float y = (character / numbLetters) * scale;

	// setting up the 4 corners around the letter we want
	glm::vec2 up_left    = glm::vec2(x, y);
	glm::vec2 up_right   = glm::vec2(x + scale, y);
	glm::vec2 down_right = glm::vec2(x + scale, y + scale);
	glm::vec2 down_left  = glm::vec2(x, y + scale);

	// upper triangle
	populateVector(uvs, up_left, down_left, up_right);

	// lower triangle
	populateVector(uvs, down_right, up_right, down_left);
}

void draw(GLsizei size)
{
	// bind shader
	glUseProgram(Text2DShaderID);

	// make sure its filling
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Text2DTextureID);
	glUniform1i(Text2DUniformID, 0);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// turn on blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw call
	glDrawArrays(GL_TRIANGLES, 0, size);

	// disable blend after drawing
	glDisable(GL_BLEND);
	
	// cleanup
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glUseProgram(0);
}

void printText2D(const char * text, int x, int y, int size)
{
	// length of the text to render
	unsigned int length = (unsigned int)strlen(text);

	// fill buffers
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;

	// find the right places to draw to and what letters from the texture are needed
	for ( unsigned int i=0 ; i<length ; i++ ){
		
		populateVerticesPerLetter(vertices, x, y, size, i);

		populateUVsPerLetter(UVs, text[i]);
	}

	// bind vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);

	// bind uv buffer
	glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);

	// now draw the text on screen
	draw((GLsizei)vertices.size());
}

void cleanupText2D()
{
	// delete buffers
	glDeleteBuffers(1, &Text2DVertexBufferID);
	glDeleteBuffers(1, &Text2DUVBufferID);

	// delete texture
	glDeleteTextures(1, &Text2DTextureID);

	// delete shader
	glDeleteProgram(Text2DShaderID);
}
