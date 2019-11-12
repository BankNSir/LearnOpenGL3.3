#version 330 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	vec3 direction;
	float cutoff;
	float outercutoff;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

// some previous texture(ignored)
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float smiley_intensity;

void main()
{
	// FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(TexCoord.x, TexCoord.y)), smiley_intensity);
	// vec4 textureMap = texture(texture2, TexCoord


	// Phong Shading	1. ambient, 2. diffuse, 3.specular
	// ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diffStrength = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diffStrength * light.diffuse * vec3(texture(material.diffuse, TexCoord));

	// specular
	vec3 viewDir =	normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoord));

	// additional effects
	float distance = length(FragPos - light.position);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutoff - light.outercutoff;
	float intensity = (theta - light.outercutoff)/ epsilon;

	diffuse *= attenuation;
	specular *= attenuation;
	diffuse *= intensity;
	specular *= intensity;
	// result
	vec3 result = (ambient + diffuse + specular);

	FragColor = vec4(result, 1.0f);
}