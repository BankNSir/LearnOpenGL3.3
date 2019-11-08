#version 330 core

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;
out vec3 result;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectMat;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
	// ambient
	vec3 ambient = material.ambient * light.ambient;

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diffStrength = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diffStrength * material.diffuse * light.diffuse;

	// specular
	vec3 viewDir =	normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * material.specular * light.specular;

	gl_Position = projectMat * viewMat * modelMat * vec4(aPos, 1.0f);

	// in global coordinate
	FragPos = vec3(modelMat * vec4(aPos, 1.0f));
	Normal = mat3(transpose(inverse(modelMat))) * aNormal;
	result = ambient + diffuse + specular;

}