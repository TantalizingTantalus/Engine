

#version 410 core

layout(location = 0) in vec3 aPos;   
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord; 
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 Normal;
} vs_out;  

vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vs_out.TexCoords = aTexCoord;

    // Normal = mat3(transpose(inverse(model))) * aNormal;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    //mat3 normalMatrix = mat3(model);
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    float handedness = dot(cross(aTangent, aNormal), aBitangent) < 0.0 ? -1.0 : 1.0;
vec3 B = handedness * cross(N, T);
    //vec3 B = cross(N, T);
    vs_out.Normal = aNormal;
    vs_out.TBN = transpose(mat3(T, B, N));

    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}

