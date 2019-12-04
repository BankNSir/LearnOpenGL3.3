#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
	// for preventing bug from using alpha value use only RGB components of texture
	// vec4 texColor = texture(texture1, TexCoords);

	// if (texColor.a <= 0.1)
	// 	discard;

	FragColor = texture(texture1, TexCoords);	
}