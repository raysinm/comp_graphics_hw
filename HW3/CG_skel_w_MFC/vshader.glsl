#version 150

//Input
in vec3 vPosition;
in vec3 vColor;     //for non-uniform material
uniform mat4 modelview;
uniform vec3 uniformColor_emissive;
uniform vec3 uniformColor_diffuse;
uniform vec3 uniformColor_specular;

//Output
out vec3 colorOfVertex;


//Local variables:
vec4 pos;


void main()
{
    pos = vec4(vPosition, 1.0);
    
//    gl_Position = modelview * pos;
    gl_Position =  pos;

//    colorOfVertex = uniformColor_diffuse;
    colorOfVertex = vColor;
}
