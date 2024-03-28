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





//Input
in vec3 vPosition;
in vec3 vColor;     //for non-uniform material
in vec3 vn;
in vec3 fn;


uniform int algo_shading;
uniform int displayBBox;
uniform int displayVnormal;
uniform int displayFnormal;
uniform int numLights;
uniform float vnFactor;
uniform float fnFactor;
uniform mat4 modelview;
uniform mat4 modelview_normals;
uniform mat4 projection;
uniform vec3 wireframeColor;
uniform vec3 uniformColor_emissive;
uniform vec3 uniformColor_diffuse;
uniform vec3 uniformColor_specular;

//Output
out vec3 colorOfVertex;

//Local:
vec4 pos;
vec4 res;
int vertexIndex;

void main()
{
    pos = vec4(vPosition, 1);
    vertexIndex = gl_VertexID;
    
    res = projection * (modelview * pos);
    if(displayBBox == 1)
    {
        colorOfVertex = vec3(0, 1, 0);  // green
    }
    else if (displayFnormal == 1)
    {
        colorOfVertex = vec3(0, 0, 1);  // blue
        if(vertexIndex % 2 == 1) 
        {
            vec4 normalDir = normalize(projection * normalize(modelview_normals * vec4(fn,1)));
            vec4 startPoint = res / res.w;
            res = startPoint + (normalDir * fnFactor);
            res.w = 1;
        }
    }
    else if (displayVnormal == 1)
    {
        colorOfVertex = vec3(1, 0, 0);
        if(vertexIndex % 2 == 1) 
        {
            vec4 normalDir = normalize(projection * normalize(modelview_normals * vec4(vn,1)));
            vec4 startPoint = res / res.w;
            res = startPoint + (normalDir * vnFactor);
            res.w = 1;
        }
    }
    else
    {
        if(algo_shading == 0) //wireframe
        {
            colorOfVertex = wireframeColor;
        }
        else if(algo_shading == 1) //flat shading
        {
            //test
            colorOfVertex = vec3(1,0,0);
        }
        else if(algo_shading == 2) //Gouraud shading
        {
            //test
            colorOfVertex = vec3(0,1,0);
        }
        else if(algo_shading == 3) //Phong shading
        {
            //test
            colorOfVertex = vec3(0,0,1);
        }

    }

    gl_Position = res;
}
