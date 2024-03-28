#version 150

struct LightProperties
{
    vec4 position;
    vec4 dir;
    vec4 color;
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


in vec3 outputColor;
flat in vec3 flat_outputColor;

uniform vec3 wireframeColor;
uniform int algo_shading;
uniform int displayBBox;
uniform int displayVnormal;
uniform int displayFnormal;
uniform int numLights;

out vec4 FragColor;

void main()
{
    if(algo_shading == 0) //WireFrame
    {
	    FragColor = vec4(outputColor, 1);
    }
	else
    {
        if(algo_shading == 1)       // flat shading
        {
            FragColor = vec4(flat_outputColor, 1);
        }
        else if(algo_shading == 2)  // gououord shading
        {
            FragColor = vec4(outputColor, 1);
        }
        else if(algo_shading == 3)  // Phong shading
        {
            FragColor = vec4(0,1,0,1);
        }
    }
} 

