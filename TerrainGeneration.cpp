#include <iostream>
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>

#include "DiamondSquare.h"
#include "getbmp.h"

#pragma comment(lib, "glew32.lib") 

using namespace std;

using namespace glm;

// Size of the terrain
const int MAP_SIZE = 2049;

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 1024;


struct Vertex
{
	float coords[4];
	float normals[3];
	float texcoords[2];
};

struct Matrix4x4
{
	float entries[16];
};

static mat4 projMat = mat4(1.0);
static mat3 normalMat = mat3(1.0);

static const Matrix4x4 IDENTITY_MATRIX4x4 =
{
	{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	}
};

struct Material
{
	vec4 ambRefl;
	vec4 difRefl;
	vec4 specRefl;
	vec4 emitCols;
	float shininess;
};

struct Light
{
	vec4 ambCols;
	vec4 difCols;
	vec4 specCols;
	vec4 coords;
};

static const vec4 globAmb = vec4(0.9f, 0.9f, 0.9f, 1.0f);

static enum buffer { SQUARE_VERTICES };
static enum object { SQUARE };

// Globals
mat4 modelViewMat = mat4(1.0);
vec3 eyePos = vec3(0.0, 0.0, 100.0);
vec3 upVector = vec3(0.0, 1.0, 0.0);
vec3 lookPos = vec3(0.0, 0.0, 1.0);
float cameraTheta = 0.0f;
float cameraGama = 0.0f;

 Vertex terrainVertices[MAP_SIZE * MAP_SIZE] = {};

const int numStripsRequired = MAP_SIZE - 1;
const int verticesPerStrip = 2 * MAP_SIZE;

unsigned int terrainIndexData[numStripsRequired][verticesPerStrip];

static BitMapFile *image[1];

//Not used
//unsigned int newTerrainIndexData[numStripsRequired * verticesPerStrip];
static unsigned int
programId,
vertexShaderId,
fragmentShaderId,
modelViewMatLoc,
projMatLoc,
buffer[1],
vao[1],
texture[1],
grassTexLoc;

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

static const Material terrainFandB =
{
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(0.0, 0.0, 0.0, 1.0),
	50.0f
};

static const Light light0 =
{
	vec4(0.0, 0.0, 0.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 0.0, 0.0)
};


// Function to read text file, used to read shader files
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



// Function to replace GluPerspective provided by NEHE
// http://nehe.gamedev.net/article/replacement_for_gluperspective/21002/
void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	const GLdouble pi = 3.1415926535897932384626433832795;
	GLdouble fW, fH;

	fH = tan(fovY / 360 * pi) * zNear;
	fW = fH * aspect;

	projMat = frustum(-fW, fW, -fH, fH, zNear, zFar);
}

// Initialization routine.
void setup()
{
	glEnable(GL_DEPTH_TEST);

	srand(1213);
	
	// Initialise terrain - set values in the height map to 0
	//CHANGED: FROM DEFAULT ARRAY TO VECTOR
	vector< vector <float> > terrain(MAP_SIZE, vector<float>(MAP_SIZE,1));
	for (int x = 0; x < MAP_SIZE; x++)
	{
		for (int z = 0; z < MAP_SIZE; z++)
		{
			terrain[x][z] = 0;
		}
	}

	// TODO: Add your code here to calculate the height values of the terrain using the Diamond square algorithm
	ds::diamondSquareSetup(terrain);
	
	ds::diamondSquare(terrain, 1024, 100);

	float fTextureS = float(MAP_SIZE)*0.1f;
	float fTextureT = float(MAP_SIZE)*0.1f;

	// Intialise vertex array
	int i = 0;

	for (int z = 0; z < MAP_SIZE; ++z)
	{
		for (int x = 0; x < MAP_SIZE; ++x)
		{
			// Set the coords (1st 4 elements) and a default colour of black (2nd 4 elements) 

			vec3 temp1 = vec3((float)x, terrain[x][z], (float)z);

			vec3 temp2 = vec3(0.0, 0.0, 0.0);
			vec3 temp3 = vec3(0.0, 0.0, 0.0);

			if (x == MAP_SIZE - 1 && z == MAP_SIZE - 1)
			{
				temp2 = vec3(-1, terrain[x - 1][z], 0);
				temp3 = vec3(0, terrain[x][z - 1], -1);
			}
			else if (x == MAP_SIZE - 1)
			{
				temp2 = vec3(-1, terrain[x - 1][z], 0);
				temp3 = vec3(0, terrain[x][z + 1], 1);
			}
			else if (z == MAP_SIZE - 1)
			{
				temp2 = vec3(1, terrain[x + 1][z], 0);
				temp3 = vec3(0, terrain[x][z - 1], -1);
			}
			else
			{
				temp2 = vec3(1, terrain[x + 1][z], 0);
				temp3 = vec3(0, terrain[x][z + 1], 1);
			}
			

			vec3 toCross1 = normalize(temp1 - temp2);
			vec3 toCross2 = normalize(temp1 - temp3);

			vec3 normal = cross(toCross1, toCross2);

			float fScaleC = float(x) / float(MAP_SIZE - 1);
			float fScaleR = float(z) / float(MAP_SIZE - 1);

			terrainVertices[i] = { { (float)x, terrain[x][z], (float)z, 1.0 }, { normal.x, normal.y, normal.z }, {fTextureS * fScaleC, fTextureT * fScaleR} };
			i++;
		}
	}

	// Now build the index data 
	i = 0;
	for (int z = 0; z < MAP_SIZE - 1; z++)
	{
		i = z * MAP_SIZE;
		for (int x = 0; x < MAP_SIZE * 2; x += 2)
		{
			terrainIndexData[z][x] = i;
			i++;
		}
		for (int x = 1; x < MAP_SIZE * 2 + 1; x += 2)
		{
			terrainIndexData[z][x] = i;
			i++;
		}
	}

	glClearColor(1.0, 1.0, 1.0, 0.0);

	// Create shader program executable - read, compile and link shaders
	char* vertexShader = readTextFile("vertexShader.glsl");
	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, (const char**)&vertexShader, NULL);
	glCompileShader(vertexShaderId);

	char* fragmentShader = readTextFile("fragmentShader.glsl");
	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, (const char**)&fragmentShader, NULL);
	glCompileShader(fragmentShaderId);

	cout << "VERTEX: " << endl;
	shaderCompileTest(vertexShaderId);

	cout << "FRAGMENT: " << endl;
	shaderCompileTest(fragmentShaderId);

	programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);
	glUseProgram(programId);
	///////////////////////////////////////


	//////////////TEXTURES///////////////////
	// Load the image.
	image[0] = getbmp("terrain_texture.bmp");
	// Create texture id.
	glGenTextures(1, texture);
	// Bind grass image.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	grassTexLoc = glGetUniformLocation(programId, "grassTex");
	glUniform1i(grassTexLoc, 0);
	/////////////////////////////////////////
	// Create vertex array object (VAO) and vertex buffer object (VBO) and associate data with vertex shader.
	glGenVertexArrays(1, vao);
	glGenBuffers(1, buffer);
	glBindVertexArray(vao[SQUARE]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[SQUARE_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), (GLvoid*)sizeof(terrainVertices[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]),
		(GLvoid*)(sizeof(terrainVertices[0].coords) + sizeof(terrainVertices[0].normals)));
	glEnableVertexAttribArray(2);
	///////////////////////////////////////

	// Obtain projection matrix uniform location and set value.
	projMatLoc = glGetUniformLocation(programId, "projMat");
	perspectiveGL(60, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1, 100);
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.ambRefl"), 1, &terrainFandB.ambRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.difRefl"), 1, &terrainFandB.difRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.specRefl"), 1, &terrainFandB.specRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.emitCols"), 1, &terrainFandB.emitCols[0]);
	glUniform1f(glGetUniformLocation(programId, "terrainFandB.shininess"),	terrainFandB.shininess);

	glUniform4fv(glGetUniformLocation(programId, "light0.ambCols"), 1, &light0.ambCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.difCols"), 1, &light0.difCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.specCols"), 1, &light0.specCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.coords"), 1, &light0.coords[0]);

	glUniform4fv(glGetUniformLocation(programId, "globAmb"), 1, &globAmb[0]);
	///////////////////////////////////////

	projMatLoc = glGetUniformLocation(programId, "projMat");
	projMat = glm::perspective(1.0472, 1.0, 0.1, 200000000.0);
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

	// Obtain modelview matrix uniform location and set value.
	modelViewMat = lookAt(eyePos, lookPos, upVector);
	// Move terrain into view - glm::translate replaces glTranslatef
	//modelViewMat = translate(modelViewMat, vec3(-15.5f, -20.0f, -80.0f)); // 5x5 grid
	modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

	normalMat = transpose(inverse(mat3(modelViewMat)));
	glUniformMatrix3fv(glGetUniformLocation(programId, "normalMat"), 1, GL_FALSE, value_ptr(normalMat));
	///////////////////////////////////////
/*
	Matrix4x4 modelViewMatTwo = IDENTITY_MATRIX4x4;
	modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_TRUE, modelViewMatTwo.entries);*/
}

// Drawing routine.
void drawScene(void)
{

	lookPos.x = cos(radians(cameraGama)) * sin(radians(cameraTheta));
	lookPos.y = sin(radians(cameraGama));
	lookPos.z = -cos(radians(cameraGama)) * cos(radians(cameraTheta));

	normalize(lookPos);

	modelViewMat = lookAt(eyePos, eyePos + lookPos, upVector);
	// Move terrain into view - glm::translate replaces glTranslatef
	//modelViewMat = translate(modelViewMat, vec3(-15.5f, -20.0f, -80.0f)); // 5x5 grid
	modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// For each row - draw the triangle strip
	for (int i = 0; i < MAP_SIZE - 1; i++)
	{
		glDrawElements(GL_TRIANGLE_STRIP, verticesPerStrip, GL_UNSIGNED_INT, terrainIndexData[i]);
	}

	glFlush();
	//glutSwapBuffers();
	glutPostRedisplay();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case'w':
		eyePos +=  lookPos * 10.0f;
		break;
	case 's':
		eyePos += lookPos * -10.0f;
		break;
	case 'a':
		eyePos += cross(upVector, lookPos) * 10.0f;
		break;
	case 'd':
		eyePos += cross(upVector, lookPos) * -10.0f;
		break;
	case 'e':
		cameraTheta += 1.0;
		break;
	case 'q':
		cameraTheta += -1.0;
		break;
	case 'r':
		eyePos += upVector * 1.0f;
		break;
	case 'f':
		eyePos += upVector * -1.0f;
		break;
	case 'z':
		cameraGama += 1.0;
		break;
	case 'x':
		cameraGama += -1.0;
		break;
	default:
		break;
	}
}

// Main routine.
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	// Set the version of OpenGL (4.2)
	glutInitContextVersion(4, 2);
	// The core profile excludes all discarded features
	glutInitContextProfile(GLUT_CORE_PROFILE);
	// Forward compatibility excludes features marked for deprecation ensuring compatability with future versions
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("TerrainGeneration");

	// Set OpenGL to render in wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}
