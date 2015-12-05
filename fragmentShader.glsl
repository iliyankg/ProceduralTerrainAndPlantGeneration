#version 420 core

smooth in vec4 colorsExport;

uniform sampler2D grassTex;

in vec2 texCoordsExport;
out vec4 colorsOut;

void main(void)
{
   vec4 fieldTexColor = texture(grassTex, texCoordsExport);
   colorsOut = fieldTexColor * colorsExport * 2.0f;
}