#version 330 core

layout (location = 0) out vec3 gPos;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;


in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

void main()
{            
	// copy to gPos
	gPos = fs_in.FragPos;
	// copy to gNormal
	gNormal = normalize(fs_in.Normal);
	// copy to Albedo and specular
	gAlbedoSpec.rgb = texture(diffuseTexture, fs_in.TexCoords).rgb;
	gAlbedoSpec.a = texture(specularTexture, fs_in.TexCoords).r;
}