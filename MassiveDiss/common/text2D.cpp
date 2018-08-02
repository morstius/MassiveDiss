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
	// load in texture
	Text2DTextureID = loadDDS(texturePath);

	// initialize the VBO
	glGenBuffers(1, &Text2DVertexBufferID);
	glGenBuffers(1, &Text2DUVBufferID);

	// initialize the shaders
	Text2DShaderID = LoadShaders( "shaders/TextVertexShader.vertexshader", "shaders/TextVertexShader.fragmentshader");

	// get handle on shaders
	Text2DUniformID = glGetUniformLocation( Text2DShaderID, "texSampler" );
}

void populateVector(std::vector<glm::vec2>& vec, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
	vec.push_back(a);
	vec.push_back(b);
	vec.push_back(c);
}

void populateVerticesPerLetter(std::vector<glm::vec2>& vertices, const int x, const int y, const unsigned int size, const int letter)
{
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
	int numbLetters = 16;
	float scale = 1.0f / (float)numbLetters;
	float x = (character % numbLetters) * scale;
	float y = (character / numbLetters) * scale;

	glm::vec2 up_left = glm::vec2(x, y);
	glm::vec2 up_right = glm::vec2(x + scale, y);
	glm::vec2 down_right = glm::vec2(x + scale, (y + scale));
	glm::vec2 down_left = glm::vec2(x, (y + scale));

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw call
	glDrawArrays(GL_TRIANGLES, 0, size);

	// disable blend after drawing
	glDisable(GL_BLEND);
}

void printText2D(const char * text, int x, int y, int size)
{
	// length of the text to render
	unsigned int length = (unsigned int)strlen(text);

	// fill buffers
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;

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

	draw((GLsizei)vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

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
