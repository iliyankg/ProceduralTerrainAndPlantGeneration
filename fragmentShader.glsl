#version 420 core

#define _SQUARE 0
#define _SKY 1
#define _CLOUD 2
#define _TREE 3
#define _LEAF 4

smooth in vec4 colorsExport;

uniform sampler2D grassTex;
uniform int switchOn;

in vec2 texCoordsExport;
in float height;

out vec4 colorsOut;

void main(void)
{
   vec4 fieldTexColor = texture(grassTex, texCoordsExport);
   if(switchOn == _CLOUD)
   {
	colorsOut = vec4(1.0, 1.0, 1.0, height / 150.0f);
   }
   else if(switchOn == _TREE || switchOn == _LEAF)
   {
	colorsOut = colorsExport;
   }
   else
   {
	 colorsOut = fieldTexColor * colorsExport * 2.0f;
   }
   
}