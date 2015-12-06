#pragma once

#include <iostream>
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>

//=============================UTILS=======================//
char* readTextFile(char* aTextFile)
{
	FILE* filePointer = fopen(aTextFile, "rb");
	char* content = NULL;
	long numVal = 0;

	fseek(filePointer, 0L, SEEK_END);
	numVal = ftell(filePointer);
	fseek(filePointer, 0L, SEEK_SET);
	content = (char*)malloc((numVal + 1) * sizeof(char));
	fread(content, 1, numVal, filePointer);
	content[numVal] = '\0';
	fclose(filePointer);
	return content;
}

void shaderCompileTest(GLuint shader)
{
	GLint result = GL_FALSE;
	int logLength;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<GLchar> vertShaderError((logLength > 1) ? logLength : 1);
	glGetShaderInfoLog(shader, logLength, NULL, &vertShaderError[0]);
	std::cout << &vertShaderError[0] << std::endl;
}

inline float getRand(float range)
{
	//return rand() % 3 - 1;
	return (((float)rand() / (float)RAND_MAX) * range) + 180.0f;

}

inline int getRandRange(int range)
{

	return rand() % range;

}

inline float getMinusPlusRand()
{
	return (((float)rand() / (float)RAND_MAX) * 2) - 1;
}

int offsetForBranches(int level, int max)
{
	if (level == max)
		return pow(2, level);

	return pow(2, level) + offsetForBranches(level + 1, max);
}
/////////////////////////////////////////////////////////////
