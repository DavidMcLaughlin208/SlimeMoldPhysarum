// fragment shader

#version 440

out vec4 outputColor;
uniform int screenWidth;
uniform int screenHeight;

struct Cell {
    vec4 val;
};

layout(binding = 2) buffer newTrail {
	Cell newTrailMap[];
};

void main()
{
    float x = gl_FragCoord.x; // screenWidth;
    float y = gl_FragCoord.y; // screenHeight;
    int index = int(x) + int(int(y) * screenWidth);
    vec4 val = newTrailMap[index].val;

    outputColor = vec4(val.r, 0, 0, 1);
}