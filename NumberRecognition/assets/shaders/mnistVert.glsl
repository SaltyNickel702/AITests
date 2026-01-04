#version 430 core
layout (location = 0) in vec2 relPos;

uniform vec2 screenDim;
uniform vec2 pos;
uniform vec2 dim;

out vec2 UV;

void main ()
{
	vec2 trs = (vec2(relPos.x * dim.x, relPos.y * dim.y) + pos) / screenDim;
	vec2 clip = (trs * 2 - 1) * vec2(1,-1);
	gl_Position = vec4(clip,0,1);

	UV = relPos;
}