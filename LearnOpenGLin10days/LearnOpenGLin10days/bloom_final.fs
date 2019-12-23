#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D normalScene;
uniform sampler2D bloomScene;
uniform float exposure;

void main()
{             
	float gamma = 2.2f;
    vec3 Color = texture(normalScene, TexCoords).rgb;
	vec3 bloomColor = texture(bloomScene, TexCoords).rgb;
	// additive combination
	vec3 result = Color + 2 * bloomColor;
	// hdrEffect
	result = vec3(1.0f) - exp(-result * exposure);
	// gamma correction
	result = pow(result, vec3(1.0f/gamma));
	
	FragColor = vec4(result, 1.0f);
}

