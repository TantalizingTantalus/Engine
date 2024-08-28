#version 330 core
out vec4 FragColor;


void main()
{


    // Output the final color
    FragColor = vec4(.004f, 0.28f, .26f, 1.0);

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