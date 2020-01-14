#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D texture1;
  
uniform bool horizontal;

float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec3 result = vec3(0.0f);
	
	// Offset texture coordinates
	vec2 offset = 1.0f / textureSize(texture1, 0);
	vec2 texCoordOffSet = vec2(0.0f);

	// if horizontal
	// blur horizontally
	if (horizontal)
	{
		texCoordOffSet = vec2(offset.x, 0.0f);
	}
	// else vertical
	// blur vertically
	else
	{
		texCoordOffSet = vec2(0.0f, offset.y);
	}

	// center of sample
	result += texture(texture1, TexCoords).rgb * weight[0];

	// for loop 5 times
	for (int i = 1; i < 5; i++)
	{
		// blurring
		result += texture(texture1, TexCoords + texCoordOffSet * i).rgb * weight[i]; 
		result += texture(texture1, TexCoords - texCoordOffSet * i).rgb * weight[i];
	}

	FragColor = vec4(result, 1.0f);
}