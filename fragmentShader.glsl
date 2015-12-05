#version 420 core

#define _SQUARE 0
#define _SKY 1
#define _CLOUD 2

smooth in vec4 colorsExport;

uniform sampler2D grassTex;
uniform int switchOn;

in vec2 texCoordsExport;

out vec4 colorsOut;

void main(void)
{
   vec4 fieldTexColor = texture(grassTex, texCoordsExport);
   if(switchOn == _CLOUD)
   {
	colorsOut = vec4(1.0, 1.0, 1.0, 0.5);
   }
   else
   {
	 colorsOut = fieldTexColor * colorsExport * 2.0f;
   }
   
}