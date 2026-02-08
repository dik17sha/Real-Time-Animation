#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords; // Changed to vec3 for cubemap sampling

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    // Remove translation from the view matrix so skybox stays centered
    mat4 viewNoTranslation = mat4(mat3(view)); 
    vec4 pos = projection * viewNoTranslation * vec4(aPos, 1.0);
    
    // xyww trick: depth will always be 1.0 (the far plane)
    gl_Position = pos.xyww;
}