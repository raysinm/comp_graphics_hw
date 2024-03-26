#version 150

//Input
in vec3 vPosition;
in vec3 vColor;     //for non-uniform material

uniform int algo_shading;
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

    if(algo_shading == 0)
    {
        colorOfVertex = wireframeColor;
    }
    
}
