#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

uniform vec3 lightColor;

vec3 PhongCalculation();

void main()
{            
	vec3 result = lightColor;    
    
    FragColor = vec4(result, 1.0f);

	// keep high lumimnance fragment
	if (dot(result, vec3(0.2126f, 0.7152f, 0.0722f)) > 1.0f)
	{
		BrightColor = vec4(result, 1.0f);
	}
	else
	{
		BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}
