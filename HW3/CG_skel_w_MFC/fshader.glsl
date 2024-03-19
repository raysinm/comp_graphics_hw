#version 150

in vec3 color_debug;

out vec4 FragColor;

void main()
{ 
	FragColor = vec4(color_debug, 1);
} 

