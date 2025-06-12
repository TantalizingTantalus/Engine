#version 410 core
out vec4 FragColor;

bool DEBUG_SINGLE_COLOR;

void main()
{
	if(!DEBUG_SINGLE_COLOR)
	{
		FragColor = vec4(1.0f, .6f, 0.0f, 1.0f);	
	}else{
		FragColor = vec4(1.0f, .6f, 0.0f, 1.0f);	
	}
}