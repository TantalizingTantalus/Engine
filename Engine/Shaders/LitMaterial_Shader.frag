#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform bool DEBUG_NORMAL;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{

    float ambientStrength = .1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
  // Sample the diffuse texture
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 finalColor = diffuseColor * result;

    FragColor = vec4(finalColor, 1.0);
    // Output the final color
    if(DEBUG_NORMAL)
    {
        FragColor = vec4(Normal, 1.0);
    }
    

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