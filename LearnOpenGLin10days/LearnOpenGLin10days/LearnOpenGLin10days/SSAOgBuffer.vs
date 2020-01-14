#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vs_out.FragPos = vec3(view * model * vec4(aPos, 1.0f));
	mat4 normalModelMatrix = inverse(transpose(model));
	mat4 normalViewMatrix = inverse(transpose(view));
	vs_out.Normal = vec3(normalViewMatrix * normalModelMatrix * vec4(aNormal, 0.0f));
	vs_out.TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}  