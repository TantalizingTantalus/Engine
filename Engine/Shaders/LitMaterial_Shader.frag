#version 410 core

out vec4 FragColor;
out vec4 PickingColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 Normal;
} fs_in;  


struct Material {
   sampler2D texture_diffuse;
    sampler2D texture_normal;
    sampler2D texture_specular;
    float shininess;
    float specularIntensity;
    bool hasNormal;
    bool hasSpecular;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 color;
    float intensity;
    bool inUse;
};

struct PointLight {
    vec3 position;
    float intensity;
    
    float constant;
    float linear;
    float quadratic;

	vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


#define NR_POINT_LIGHTS 100


uniform vec3 viewPos;
uniform vec3 objectColor = vec3(1.0f, 0.5f, 0.31f);
uniform int entityID;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;
uniform bool DEBUG_NORMAL;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    

    vec3 norm = normalize(fs_in.Normal);
    if (material.hasNormal) {
        vec3 tangentNormal = texture(material.texture_normal, fs_in.TexCoords).rgb;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        norm =  normalize(fs_in.TBN * tangentNormal); 
    }else
    {    
        norm = norm * 2.0 - 1.0;
    }
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0.0);
    
    // Directional Light
    if(dirLight.inUse)
    {
        result += CalcDirLight(dirLight, norm, viewDir);
    }


    // Point Lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        result += CalcPointLight(pointLights[i], norm, fs_in.FragPos, viewDir);    
    }

   float gamma = 2.2;

   if (DEBUG_NORMAL)
    {
        FragColor = vec4(norm * 2.0 - 1.0, 1.0);
    }
    else
    {
        FragColor = vec4(result, 1.0);
        //FragColor.rgb = pow(result.rgb, vec3(1.0/gamma));
    }
    

    PickingColor = vec4(
        float(entityID % 256) / 255.0,
        float((entityID / 256) % 256) / 255.0,
        float((entityID / 65536) % 256) / 255.0,
        1.0
    );
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = material.specularIntensity * pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.intensity * light.ambient * light.color * vec3(texture(material.texture_diffuse, fs_in.TexCoords));
    vec3 diffuse =  light.intensity * light.diffuse * light.color * diff * vec3(texture(material.texture_diffuse, fs_in.TexCoords));
    vec3 specular =   light.specular * light.color * spec * vec3(texture(material.texture_diffuse, fs_in.TexCoords));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // Calculate light direction
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * light.color;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = spec * material.specularIntensity * light.specular * light.color;

    // Attenuation calculation
//    float distance = length(light.position - fragPos);
//    float attenuation = 1.0 / (light.constant + light.linear * distance + 
//                               light.quadratic * (distance * distance));
                               
    // Ambient component
    vec3 ambient = light.ambient * light.color;

     //Combine results with attenuation
    ambient *= light.intensity;
    diffuse *= light.intensity;
    specular *= light.intensity;

    // Apply texture sampling (optional)
    vec3 texColor = vec3(texture(material.texture_diffuse, fs_in.TexCoords));
    vec3 result = (ambient + diffuse + specular) * texColor;

    return result;

}


