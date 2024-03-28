#version 150

struct LightProperties
{
    vec3 position;
    vec3 dir;
    vec3 color;
    float La;
    float Ld;
    float Ls;
    int type;  // Ambient=0 / Point=1 / parallel=2
};

// Declare a uniform block for the light properties
layout(std140) uniform Lights
{
    LightProperties lights[10]; // Maximum number of lights
};

in vec3 colorOfVertex;

uniform int algo_shading;   // isnt it already global?
uniform vec3 wireframeColor;

out vec4 FragColor;

void main()
{
    if(algo_shading == 0)
	    FragColor = vec4(wireframeColor, 1);
	else
        FragColor = vec4(lights[0].color, 1);
} 

