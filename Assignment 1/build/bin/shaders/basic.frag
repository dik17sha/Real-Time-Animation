#version 330 core
out vec4 FragColor;

in vec3 TexCoords; // Direction vector representing a point on the cube

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}