#version 330 core
out vec4 FragColor;
out vec4 PickingColor;

uniform int entityID; // Uniform for object ID

void main()
{
    FragColor = vec4(1.0); // set all 4 vector values to 1.0
     PickingColor = vec4(
        float(entityID % 256) / 255.0,
        float((entityID / 256) % 256) / 255.0,
        float((entityID / 65536) % 256) / 255.0,
        1.0
    );
}