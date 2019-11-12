#version 330 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;
};

struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct DirLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform Material material;

uniform DirLight dirLight;
uniform PointLight pointLights[4];
uniform SpotLight spotLight;
uniform bool isFlashLightOn;

// some previous texture(ignored)
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float smiley_intensity;

// function declaration
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir); 
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 result = vec3(0.0f);
	vec3 viewDir =	normalize(viewPos - FragPos);

	result += CalcDirLight(dirLight, Normal, viewDir);

	// 4 point lights
	for (int i = 0; i < 4; i++)
	{
		result += CalcPointLight(pointLights[i], Normal, FragPos, viewDir);
	}

	if (isFlashLightOn)
	{
		result += CalcSpotLight(spotLight, Normal, FragPos, viewDir);
	}

	FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	// ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

	// diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-light.direction);
	float diffStrength = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diffStrength * light.diffuse * vec3(texture(material.diffuse, TexCoord));

	// specular
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoord));

	return vec3(ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	// ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

	// diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light.position - fragPos);
	float diffStrength = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diffStrength * light.diffuse * vec3(texture(material.diffuse, TexCoord));

	// specular
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoord));

	// additional effects
	float distance = length(fragPos - light.position);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	diffuse *= attenuation;
	specular *= attenuation;

	//float theta = dot(lightDir, normalize(-light.direction));
	//float epsilon = light.cutoff - light.outercutoff;
	//float intensity = (theta - light.outercutoff)/ epsilon;
	//diffuse *= intensity;
	//specular *= intensity;

	return vec3(ambient + diffuse + specular);
}

// ****
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	// ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

	// diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light.position - fragPos);
	float diffStrength = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diffStrength * light.diffuse * vec3(texture(material.diffuse, TexCoord));

	// specular
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoord));

	// additional effects
	float distance = length(fragPos - light.position);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * distance * distance);


	diffuse *= attenuation;
	specular *= attenuation;

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff)/ epsilon, 0.0f, 1.0f);

	diffuse *= intensity;
	specular *= intensity;

	return vec3(ambient + diffuse + specular);
}