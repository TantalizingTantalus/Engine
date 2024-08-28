#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

void main()
{

  // Sample the diffuse texture
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;

    // Output the final color
    FragColor = vec4(diffuseColor, 1.0);

}

//// Fragment Shader
//#version 330 core
//in vec2 TexCoord;
//out vec4 FragColor;
//
//void main()
//{
//    //FragColor = vec4(TexCoord, 0.0, 1.0);
//    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
//}