#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D CBOTexture;

void main()
{
	float gamma = 2.2f;
	// add reinheart tone mapping : y = x/(x+1);
	vec3 Color = texture(CBOTexture, TexCoords).rgb;
	Color = 1 - exp(-Color * 0.1f);
	
	vec3 mapped = pow(Color, vec3(1.0f/gamma));
	FragColor = vec4(mapped, 1.0f);
}  