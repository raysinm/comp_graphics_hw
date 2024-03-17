#version 150

#this is yonatan comment
in  vec2 vTexCoord;
in  vec2 vPosition;


out vec2 texCoord;

void main()
{
    gl_Position.xy = vPosition;
    gl_Position.z=0;
    gl_Position.w=1;
    texCoord = vTexCoord;
}
