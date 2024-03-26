#version 150

//Input
in vec3 vPosition;
in vec3 vColor;     //for non-uniform material
in vec3 vn;
in vec3 fn;


uniform int algo_shading;
uniform int displayBBox;
uniform int displayVnormal;
uniform int displayFnormal;
uniform float vnFactor;
uniform float fnFactor;
uniform mat4 modelview;
uniform mat4 modelview_normals;
uniform mat4 projection;
uniform vec3 uniformColor_emissive;
uniform vec3 uniformColor_diffuse;
uniform vec3 uniformColor_specular;
uniform vec3 wireframeColor;

//Output
out vec3 colorOfVertex;


//Local:
vec4 pos;
vec4 res;
int vertexIndex;

void main()
{
    pos = vec4(vPosition, 1.0);
    vertexIndex = gl_VertexID;
    
    res = projection * (modelview * pos);

    if(displayBBox == 1)
    {
        colorOfVertex = vec3(0, 1, 0);
    }
    else if (displayFnormal == 1)
    {
        colorOfVertex = vec3(0, 0, 1);
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

    gl_Position = res;
}
