#version 330 core

out vec4 FragColor;

void main()
{
	// FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(TexCoord.x, TexCoord.y)), smiley_intensity);
	FragColor = vec4(1.0f);
}