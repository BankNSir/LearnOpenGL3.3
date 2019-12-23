#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

// render quad

out VS_OUT
{
	vec2 TexCoords;
} vs_out;

void main()
{
	gl_Position = vec4(aPos, 1.0f);
	vs_out.TexCoords = aTexCoords;
}