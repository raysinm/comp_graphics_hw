#version 150

//Input
in vec3 vPosition;
in vec3 vColor;     //for non-uniform material

uniform int algo_shading;
uniform int displayBBox;
uniform int displayVnormal;
uniform int displayFnormal;
uniform mat4 modelview;
uniform mat4 projection;
uniform vec3 uniformColor_emissive;
uniform vec3 uniformColor_diffuse;
uniform vec3 uniformColor_specular;
uniform vec3 wireframeColor;

//Output
out vec3 colorOfVertex;

void main()
{
    vec4 pos = vec4(vPosition, 1.0);
    vec4 res = projection * (modelview * pos);
    
    gl_Position = res;

    if(displayBBox == 1)
    {
        colorOfVertex = vec3(0, 1, 0);
    }
    else if (displayFnormal == 1)
    {
        colorOfVertex = vec3(0, 0, 1);
    }
    else if (displayVnormal == 1)
    {
        colorOfVertex = vec3(1, 0, 0);
    }
    else
    {
        if(algo_shading == 0)
        {
            colorOfVertex = wireframeColor;
        }
        else
        {
            //Debug: Default color for now...
            colorOfVertex = vec3(0.5, 0.7, 1);
        }
    }

}
