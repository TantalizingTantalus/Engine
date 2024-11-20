#version 410 core
//
//
//out vec4 FragColor;
//
//in vec2 TexCoords;
//in vec3 FragPos;
//in vec3 Normal;
//
//uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_normal1;
//uniform bool DEBUG_NORMAL;
//
//uniform vec3 lightPos;
//uniform vec3 viewPos;
//uniform vec3 lightColor;
//uniform vec3 objectColor = {1.0f, .5f, .31f};
//
//
//void main()
//{
//   
//        float ambientStrength = .1;
//        vec3 ambient = ambientStrength * lightColor;
//
//        vec3 norm = normalize(Normal);
//        vec3 lightDir = normalize(lightPos - FragPos);
//        float diff = max(dot(norm, lightDir), 0.0);
//        vec3 diffuse = diff * lightColor;
//
//        float specularStrength = 0.5;
//        vec3 viewDir = normalize(viewPos - FragPos);
//        vec3 reflectDir = reflect(-lightDir, norm);
//        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//        vec3 specular = specularStrength * spec * lightColor;
//
//        vec3 result = (ambient + diffuse + specular) * objectColor;
//      
//        vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
//        vec3 finalColor = diffuseColor * result;
//
//    
//      
//        if(DEBUG_NORMAL)
//        {
//            FragColor = vec4(norm, 1.0);
//        }else{
//            FragColor = vec4(finalColor, 1.0);
//        }
//    
//}
//
//
//
//


out vec4 FragColor;
out vec4 PickingColor; 

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform bool DEBUG_NORMAL;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor = vec3(1.0f, 0.5f, 0.31f);

uniform int entityID; 

void main()
{
    // Lighting calculations
    float ambientStrength = 0.1;
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
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 finalColor = diffuseColor * result;

    // Assign the final color to the main fragment output
    if (DEBUG_NORMAL)
    {
        FragColor = vec4(norm, 1.0);
    }
    else
    {
        FragColor = vec4(finalColor, 1.0);
    }

    // Encode the entity ID as a color
    PickingColor = vec4(
        float(entityID % 256) / 255.0,
        float((entityID / 256) % 256) / 255.0,
        float((entityID / 65536) % 256) / 255.0,
        1.0
    );

}