#version 330 core
layout(location = 0) in vec3 aPos;   // Vertex position
layout(location = 1) in vec2 aTexCoord; // Texture coordinate

out vec2 TexCoords;

uniform mat4 model;

void main()
{
    TexCoords = aTexCoord;
    gl_Position = model * vec4(aPos, 1.0);
}