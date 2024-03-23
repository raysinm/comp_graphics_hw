#version 150

in vec3 colorOfVertex;

out vec4 FragColor;

void main()
{ 
	FragColor = vec4(colorOfVertex, 1);
} 

