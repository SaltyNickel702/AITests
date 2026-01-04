#version 430 core
layout(std430, binding = 0) buffer Data {
    float pixels[];
};

uniform vec2 grid;
uniform vec2 dim;

in vec2 UV;

out vec4 color;

void main ()
{
	vec2 gridPos = floor(UV * grid);
	float i = gridPos.y * grid.x + gridPos.x;
	float shade = pixels[int(i)];
	color = vec4(vec3(shade),1);

	vec2 lineWidth = grid / dim;
	vec2 gridLines = mod(UV * grid,1);
	if (gridLines.x < lineWidth.x || gridLines.y < lineWidth.y) color = vec4(.8,.8,1,1);
}