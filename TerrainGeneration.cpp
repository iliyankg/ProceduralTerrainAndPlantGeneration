#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <time.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>

#include "DiamondSquare.h"
#include "getbmp.h"
#include "Structs.h"
#include "Trees.h"
#include "Utils.h"

#pragma comment(lib, "glew32.lib") 

using namespace std;

using namespace glm;

// Size of the terrain
const int MAP_SIZE = 2049;

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 1024;

//===========MATRICES==============//
static mat4 projMat = mat4(1.0);
mat4 translateMat = mat4(1.0);
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
/////////////////////////////////////

//======================UTILS=================================//
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
////////////////////////////////////////////////////////////////

//===============ENUMS==============//
static enum buffer { SQUARE_VERTICES, SKY_VERTICES, CLOUD_VERTICES, TREE_VERTS, LEAF_VERTS };
static enum object { SQUARE, SKY, CLOUD, TREE, LEAF };
/////////////////////////////////////

//==============================GLOBALS==============================//
Tree trees[3];

static const vec4 globAmb = vec4(0.9f, 0.9f, 0.9f, 1.0f);

mat4 modelViewMat = mat4(1.0);
vec3 eyePos = vec3(0.0, 0.0, 100.0);
vec3 upVector = vec3(0.0, 1.0, 0.0);
vec3 lookPos = vec3(0.0, 0.0, 1.0);
float cameraTheta = 0.0f;
float cameraGama = 0.0f;

Vertex terrainVertices[MAP_SIZE * MAP_SIZE] = {};
Vertex skyVertices[8] = {};

Vertex treeVertsOne[1000] = {};
Vertex treeLeafVertsOne[2000] = {};
Vertex leafVertsOne[2000];
Vertex treeVertsTwo[1000] = {};
Vertex treeLeafVertsTwo[2000] = {};
Vertex leafVertsTwo[2000];
Vertex treeVertsThree[1000] = {};
Vertex treeLeafVertsThree[2000] = {};
Vertex leafVertsThree[2000];

std::vector < Vertex > validTreeLocations;
std::vector < Vertex > finalTreeLocations;
int treeVer[100];
mat4 treeTransformMat[100];

const int numStripsRequired = MAP_SIZE - 1;
const int verticesPerStrip = 2 * MAP_SIZE;

unsigned int treeIndeciesOne[1000][2] = {};
unsigned int treeIndeciesTwo[1000][2] = {};
unsigned int treeIndeciesThree[1000][2] = {};
unsigned int terrainIndexData[numStripsRequired][verticesPerStrip];
unsigned int skyIndexData[6] = {};
////////////////////////////////////////////////////////////////////////

//===========================BUFFERS=======================//
static unsigned int
programId,
vertexShaderId,
fragmentShaderId,
modelViewMatLoc,
projMatLoc,
buffer[8],
vao[8],
ibo[3],
texture[2],
grassTexLoc,
rockTexLoc;

static BitMapFile *image[2];
//////////////////////////////////////////////////////////////

//==============SPECIALS=======================//
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
/////////////////////////////////////////////

// Initialization routine.
void setup()
{
	//=============================ENABLES & SETUP=================================//
	glEnable(GL_DEPTH_TEST);

	glClearColor(117.0/255.0, 210.0/255.0, 223.0/255.0, 0.0);

	srand(time(NULL));
	///////////////////////////////////////////////////////////////////////////////

	vector< vector <float> > terrain(MAP_SIZE, vector<float>(MAP_SIZE,1));
	for (int x = 0; x < MAP_SIZE; x++)
	{
		for (int z = 0; z < MAP_SIZE; z++)
		{
			terrain[x][z] = 0;
		}
	}

	//==============================GENERATE==============================//
	ds::diamondSquareSetup(terrain);	
	ds::diamondSquare(terrain, 1024, 100);

	trees[0] = Tree(9);
	trees[0].makeTree(0, 2, 20.0, 0);
	trees[0].makeLeaves(pow(2, trees[0].branchLevels - 1));

	trees[1] = Tree(9);
	trees[1].makeTree(0, 2, 20.0, 0);
	trees[1].makeLeaves(pow(2, trees[1].branchLevels - 1));

	trees[2] = Tree(9);
	trees[2].makeTree(0, 2, 20.0, 0);
	trees[2].makeLeaves(pow(2, trees[2].branchLevels - 1));
	///////////////////////////////////////////////////////////////////////

	float fTextureS = float(MAP_SIZE)*0.1f;
	float fTextureT = float(MAP_SIZE)*0.1f;
	// Intialise vertex array
	int i = 0;

	//=============================PopulatesVertices=====================================//
	
	//===========TERRAIN============//
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

			terrainVertices[i] = { { (float)x, terrain[x][z], (float)z, 1.0 }, { normal.x, normal.y, normal.z }, {fTextureS * fScaleC, fTextureT * fScaleR}, {} };
			i++;
		}
	}

	for (int i = 0; i <= 100; ++i)
	{
		int pointX = getRandRange(MAP_SIZE - 1);
		int pointZ = getRandRange(MAP_SIZE - 1);

		while (terrain[pointX][pointZ] > 10.0 || terrain[pointX][pointZ] < 0.0)
		{
			pointX = getRandRange(MAP_SIZE - 1);
			pointZ = getRandRange(MAP_SIZE - 1);
		}

		treeVer[i] = getRandRange(3);


		translateMat = mat4(1.0);
		
		treeTransformMat[i] = rotate(translateMat, radians((float)getRandRange(359)), vec3(0.0, 1.0, 0.0));
		treeTransformMat[i] = translate(translateMat, vec3(pointX, terrain[pointX][pointZ] - 5, pointZ));
		if (treeVer[i] == 2)
			treeTransformMat[i] = scale(treeTransformMat[i], vec3(0.5, 0.2, 0.5));
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
	//////////////////////////////////////

	//==================================TREES=====================================//
	//====================ONE======================//
	for (int i = 0; i < trees[0].indecies.size(); ++i)
	{
		treeIndeciesOne[i][0] = trees[0].indecies[i][0];
		treeIndeciesOne[i][1] = trees[0].indecies[i][1];
	}
	for (int i = 0; i < trees[0].treeVerts.size(); ++i)
	{
		treeVertsOne[i] = trees[0].treeVerts[i];
	}
	for (int i = 0; i < trees[0].leafVerts.size(); ++i)
	{
		leafVertsOne[i] = trees[0].leafVerts[i];
	}
	/////////////////////////////////////////////////
	//====================TWO======================//
	for (int i = 0; i < trees[1].indecies.size(); ++i)
	{
		treeIndeciesTwo[i][0] = trees[1].indecies[i][0];
		treeIndeciesTwo[i][1] = trees[1].indecies[i][1];
	}
	for (int i = 0; i < trees[1].treeVerts.size(); ++i)
	{
		treeVertsTwo[i] = trees[1].treeVerts[i];
	}
	for (int i = 0; i < trees[1].leafVerts.size(); ++i)
	{
		leafVertsTwo[i] = trees[1].leafVerts[i];
	}
	/////////////////////////////////////////////////
	//====================THREE====================//
	for (int i = 0; i < trees[2].indecies.size(); ++i)
	{
		treeIndeciesThree[i][0] = trees[2].indecies[i][0];
		treeIndeciesThree[i][1] = trees[2].indecies[i][1];
	}
	for (int i = 0; i < trees[0].treeVerts.size(); ++i)
	{
		treeVertsThree[i] = trees[2].treeVerts[i];
	}
	for (int i = 0; i < trees[2].leafVerts.size(); ++i)
	{
		leafVertsThree[i] = trees[2].leafVerts[i];
	}
	/////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


	//===============SKY==============//
	skyVertices[0] = 
	{ 
		{
			terrainVertices[0].coords[0],
			terrainVertices[0].coords[1] - 1000,
			terrainVertices[0].coords[2],
			terrainVertices[0].coords[3] 
		},
		{
			1.0f, 0.0f, 0.0f,
		},
		{
			terrainVertices[0].texcoords[0],
			terrainVertices[0].texcoords[1]
		},
		{}
	};
	skyVertices[1] =
	{
		{
			terrainVertices[0].coords[0],
			terrainVertices[0].coords[1] + 1000,
			terrainVertices[0].coords[2],
			terrainVertices[0].coords[3]
		},
		{
			1.0f, 0.0f, 0.0f,
		},
		{
			terrainVertices[0].texcoords[0],
			terrainVertices[0].texcoords[1]
		}
		,
		{}
	};
	skyVertices[2] =
	{
		{
			terrainVertices[MAP_SIZE - 1].coords[0],
			terrainVertices[MAP_SIZE - 1].coords[1] - 1000,
			terrainVertices[MAP_SIZE - 1].coords[2],
			terrainVertices[MAP_SIZE - 1].coords[3]
		},
		{
			1.0f, 0.0f, 0.0f,
		},
		{
			terrainVertices[MAP_SIZE - 1].texcoords[0],
			terrainVertices[MAP_SIZE - 1].texcoords[1]
		},
		{}
	};
	skyVertices[3] =
	{
		{
			terrainVertices[MAP_SIZE - 1].coords[0],
			terrainVertices[MAP_SIZE - 1].coords[1] + 1000,
			terrainVertices[MAP_SIZE - 1].coords[2],
			terrainVertices[MAP_SIZE - 1].coords[3]
		},
		{
			1.0f, 0.0f, 0.0f,
		},
		{
			terrainVertices[MAP_SIZE - 1].texcoords[0],
			terrainVertices[MAP_SIZE - 1].texcoords[1]
		},
		{}
	};

	skyVertices[4] =
	{
		{
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].coords[0],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].coords[1] - 1000,
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].coords[2],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].coords[3]
		},
		{
			1.0f, 0.0f, 0.0f,
		},
		{
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].texcoords[0],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].texcoords[1]
		},
		{}

	};
	skyVertices[5] =
	{
		{
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].coords[0],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].coords[1] + 1000,
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].coords[2],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].coords[3]
		},
		{
			1.0f, 0.0f, 0.0f,
		},
		{
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].texcoords[0],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1) - MAP_SIZE - 1].texcoords[1]
		},
		{}
	};
	skyVertices[6] =
	{
		{
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1)].coords[0],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1)].coords[1] - 1000,
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1)].coords[2],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1)].coords[3]
		},
		{
			1.0f, 0.0f, 0.0f,
		},
		{
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1)].texcoords[0],
			terrainVertices[(MAP_SIZE - 1) * (MAP_SIZE - 1)].texcoords[1]
		},
		{}
	};
	skyVertices[7] =
	{
		{
			terrainVertices[(MAP_SIZE) * (MAP_SIZE) - 1 ].coords[0],
			terrainVertices[(MAP_SIZE)* (MAP_SIZE) - 1].coords[1] + 1000,
			terrainVertices[(MAP_SIZE)* (MAP_SIZE) - 1].coords[2],
			terrainVertices[(MAP_SIZE)* (MAP_SIZE) - 1].coords[3]
		},
		{
			1.0f, 0.0f, 0.0f,
		},
		{
			terrainVertices[(MAP_SIZE)* (MAP_SIZE) - 1].texcoords[0],
			terrainVertices[(MAP_SIZE)* (MAP_SIZE) - 1].texcoords[1]
		},
		{}
	};

	skyIndexData[0] = 1;
	skyIndexData[1] = 3;
	skyIndexData[2] = 5;
	skyIndexData[3] = 3;
	skyIndexData[4] = 5;
	skyIndexData[5] = 7;
	///////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////

	//===============================SHADERS======================================//
	char* vertexShader = readTextFile("vertexShader.glsl");
	char* fragmentShader = readTextFile("fragmentShader.glsl");
	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShaderId, 1, (const char**)&vertexShader, NULL);
	glShaderSource(fragmentShaderId, 1, (const char**)&fragmentShader, NULL);
	glCompileShader(vertexShaderId);
	glCompileShader(fragmentShaderId);

	std::cout << "VERTEX: " << endl;
	shaderCompileTest(vertexShaderId);

	std::cout << "FRAGMENT: " << endl;
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
	image[1] = getbmp("sky.bmp");
	// Create texture id.
	glGenTextures(2, texture);
	// Bind grass image.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	grassTexLoc = glGetUniformLocation(programId, "grassTex");
	glUniform1i(grassTexLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->sizeX, image[1]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	rockTexLoc = glGetUniformLocation(programId, "rockTex");
	glUniform1i(rockTexLoc, 1);
	/////////////////////////////////////////

	//==================================VAO, VBO, IBO Setup============================//
	glGenVertexArrays(8, vao);
	glGenBuffers(8, buffer);
	glGenBuffers(3, ibo);

	glBindVertexArray(vao[SQUARE]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[SQUARE_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), (GLvoid*)sizeof(terrainVertices[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), (GLvoid*)(sizeof(terrainVertices[0].coords) + sizeof(terrainVertices[0].normals)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), (GLvoid*)(sizeof(terrainVertices[0].coords) + sizeof(terrainVertices[0].normals) + sizeof(terrainVertices[0].texcoords)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(vao[SKY]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[SKY_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyVertices), skyVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), (GLvoid*)sizeof(skyVertices[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), (GLvoid*)(sizeof(skyVertices[0].coords) + sizeof(skyVertices[0].normals)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), (GLvoid*)(sizeof(skyVertices[0].coords) + sizeof(skyVertices[0].normals) + sizeof(skyVertices[0].texcoords)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(treeIndeciesOne), treeIndeciesOne, GL_STATIC_DRAW);
	glBindVertexArray(vao[2]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(treeVertsOne), treeVertsOne, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)sizeof(treeVertsOne[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)(sizeof(treeVertsOne[0].coords) + sizeof(treeVertsOne[0].normals)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)(sizeof(treeVertsOne[0].coords) + sizeof(treeVertsOne[0].normals) + sizeof(treeVertsOne[0].texcoords)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(treeIndeciesTwo), treeIndeciesTwo, GL_STATIC_DRAW);
	glBindVertexArray(vao[3]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(treeVertsTwo), treeVertsTwo, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)sizeof(treeVertsOne[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)(sizeof(treeVertsOne[0].coords) + sizeof(treeVertsOne[0].normals)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)(sizeof(treeVertsOne[0].coords) + sizeof(treeVertsOne[0].normals) + sizeof(treeVertsOne[0].texcoords)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(treeIndeciesThree), treeIndeciesThree, GL_STATIC_DRAW);
	glBindVertexArray(vao[4]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(treeVertsThree), treeVertsThree, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)sizeof(treeVertsOne[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)(sizeof(treeVertsOne[0].coords) + sizeof(treeVertsOne[0].normals)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(treeVertsOne[0]), (GLvoid*)(sizeof(treeVertsOne[0].coords) + sizeof(treeVertsOne[0].normals) + sizeof(treeVertsOne[0].texcoords)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(vao[5]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(leafVertsOne), leafVertsOne, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)sizeof(leafVertsOne[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)(sizeof(leafVertsOne[0].coords) + sizeof(leafVertsOne[0].normals)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)(sizeof(leafVertsOne[0].coords) + sizeof(leafVertsOne[0].normals) + sizeof(leafVertsOne[0].texcoords)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(vao[6]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(leafVertsTwo), leafVertsTwo, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)sizeof(leafVertsOne[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)(sizeof(leafVertsOne[0].coords) + sizeof(leafVertsOne[0].normals)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)(sizeof(leafVertsOne[0].coords) + sizeof(leafVertsOne[0].normals) + sizeof(leafVertsOne[0].texcoords)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(vao[7]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[7]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(leafVertsThree), leafVertsThree, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)sizeof(leafVertsOne[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)(sizeof(leafVertsOne[0].coords) + sizeof(leafVertsOne[0].normals)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(leafVertsOne[0]), (GLvoid*)(sizeof(leafVertsOne[0].coords) + sizeof(leafVertsOne[0].normals) + sizeof(leafVertsOne[0].texcoords)));
	glEnableVertexAttribArray(3);
	/////////////////////////////////////////////////////////////////////////////////

	//============================================UNIFORMS================================================//
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

	glUniform1i(glGetUniformLocation(programId, "switchOn"), SQUARE);
	///////////////////////////////////////////////////////////////////////////////////////////////////

	//================================================MATRICES=========================================//
	projMatLoc = glGetUniformLocation(programId, "projMat");
	projMat = glm::perspective(1.0472, 1.0, 0.1, 200000000.0);
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

	// Obtain modelview matrix uniform location and set value.
	modelViewMat = lookAt(eyePos, lookPos, upVector);
	modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

	normalMat = transpose(inverse(mat3(modelViewMat)));
	glUniformMatrix3fv(glGetUniformLocation(programId, "normalMat"), 1, GL_FALSE, value_ptr(normalMat));

	glUniformMatrix4fv(glGetUniformLocation(programId, "translationMatrix"), 1, GL_FALSE, value_ptr(translateMat));
	//////////////////////////////////////////////////////////////////////////////////////////////////////
}

// Drawing routine.
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//===============================LOOKAT=================================//
	lookPos.x = cos(radians(cameraGama)) * sin(radians(cameraTheta));
	lookPos.y = sin(radians(cameraGama));
	lookPos.z = -cos(radians(cameraGama)) * cos(radians(cameraTheta));
	normalize(lookPos);
	modelViewMat = lookAt(eyePos, eyePos + lookPos, upVector);
	modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));
	//////////////////////////////////////////////////////////////////////////

	//===================================================GROUND==================================//
	glUniform1i(glGetUniformLocation(programId, "switchOn"), SQUARE);
	glBindVertexArray(vao[SQUARE]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[SQUARE_VERTICES]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	translateMat = mat4(1.0);
	glUniformMatrix4fv(glGetUniformLocation(programId, "translationMatrix"), 1, GL_FALSE, value_ptr(translateMat));
	// For each row - draw the triangle strip
	for (int i = 0; i < MAP_SIZE - 1; i++)
	{
		glDrawElements(GL_TRIANGLE_STRIP, verticesPerStrip, GL_UNSIGNED_INT, terrainIndexData[i]);
	}
	/////////////////////////////////////////////////////////////////////////////////////////

	//===================================================CLOUDS==================================//
	glUniform1i(glGetUniformLocation(programId, "switchOn"), CLOUD);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	translateMat = translate(translateMat, vec3(-10000, 2000.0, -10000));
	translateMat = glm::scale(translateMat, vec3(10.0, 10.0, 10.0));
	glUniformMatrix4fv(glGetUniformLocation(programId, "translationMatrix"), 1, GL_FALSE, value_ptr(translateMat));
	
	for (int i = 0; i < MAP_SIZE - 1; i++)
	{
		glDrawElements(GL_TRIANGLE_STRIP, verticesPerStrip, GL_UNSIGNED_INT, terrainIndexData[i]);
	}
	/////////////////////////////////////////////////////////////////////////////////////////

	//==================================================TREE================================//
	glDisable(GL_BLEND);
	glUniform1i(glGetUniformLocation(programId, "switchOn"), TREE);
	for (int iterator = 0; iterator <= 100; ++iterator)
	{
		glUniformMatrix4fv(glGetUniformLocation(programId, "translationMatrix"), 1, GL_FALSE, value_ptr(treeTransformMat[iterator]));
		
		glBindVertexArray(vao[2 + treeVer[iterator]]);
		glBindBuffer(GL_ARRAY_BUFFER, buffer[2 + treeVer[iterator]]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[treeVer[iterator]]);
		for (int i = 0; i < trees[treeVer[iterator]].branchLevels; ++i)
		{
			glLineWidth(trees[treeVer[iterator]].branchLevels + 3 - i);
			if (i == 0)
				glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
			else
			{
				int offset = 2;
				if (i == 1)
					offset = pow(2, i);
				else
					offset = offsetForBranches(2, i) + 2;

				glDrawElements(GL_LINES, pow(2, i + 1), GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint)));
			}
		}

		glUniform1i(glGetUniformLocation(programId, "switchOn"), LEAF);
		glBindVertexArray(vao[5 + treeVer[iterator]]);
		glBindBuffer(GL_ARRAY_BUFFER, buffer[5 + treeVer[iterator]]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, trees[treeVer[iterator]].leafVerts.size());
	}
	/////////////////////////////////////////////////////////////////////////////////////////

	glFlush();
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
	//glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("TerrainGeneration");

	// Set OpenGL to render in wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	cout << "CONTROLS" << endl;

	cout << "Forward - W" << endl;
	cout << "Back - S" << endl;
	cout << "Strafe Left - A" << endl;
	cout << "Strafe Right - D" << endl;
	cout << "Strafe Up - R" << endl;
	cout << "Strafe Down - F" << endl << endl;

	cout << "Rotate Up - Z" << endl;
	cout << "Rotate Down - X" << endl;
	cout << "Rotate Left - Q" << endl;
	cout << "Rotate Right - E" << endl;

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}
