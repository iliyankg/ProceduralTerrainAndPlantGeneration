#version 420 core

#define _SQUARE 0
#define _SKY 1
#define _CLOUD 2
 
layout(location=0) in vec4 squareCoords;
layout(location=1) in vec3 terrainNormals;
layout(location=2) in vec2 terrainTexCoords;

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

uniform mat4 projMat;
uniform mat4 modelViewMat;
uniform mat4 translationMatrix; //TEST
uniform mat3 normalMat;

uniform int switchOn;

smooth out vec4 colorsExport;
out vec2 texCoordsExport;
out int objType;

uniform Material terrainFandB;

uniform Light light0;

uniform vec4 globAmb;

void main(void)
{
	texCoordsExport = terrainTexCoords / 2049;
	gl_Position = projMat * modelViewMat * translationMatrix * squareCoords; //TEST
	vec3 normal = normalize(normalMat * terrainNormals);
	vec3 lightDirection = normalize(vec3(light0.coords));
	colorsExport = globAmb * terrainFandB.ambRefl * max(dot(normal, lightDirection), 0.0f);
	objType = switchOn;
}