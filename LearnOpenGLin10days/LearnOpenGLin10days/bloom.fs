#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

struct Light
{
	vec3 LightPos;
	vec3 LightColor;
};

// struct Material
// {
// 	sampler2D Diffuse;
// 	sampler2D Specular;
// };
// uniform Material cubeMaterial;

uniform sampler2D diffuseTexture;
uniform vec3 viewPos;
uniform Light lights[4];

vec3 PhongCalculation(Light light);

void main()
{            
	// for each light in light array
	vec3 result = vec3(0.0f);

	for (int i = 0; i < 4; i++)
		result += PhongCalculation(lights[i]);    
	

	// result = vec3(1.0f) - 1 / (result + vec3(1.0f));
    FragColor = vec4(result, 1.0f);
	// FragColor = texture(diffuseTexture, fs_in.TexCoords);

	// keep high lumimnance fragment	
	if (dot(result, vec3(0.2126f, 0.7152f, 0.0722f)) > 1.0f)
	{
		BrightColor = vec4(result, 1.0f);
	}
	else
	{
		BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

vec3 PhongCalculation(Light light)
{
	vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
	// vec3 color = vec3(1.0f);
    vec3 normal = normalize(fs_in.Normal);
    // ambient
    vec3 ambient = 0.0f * color;
    // diffuse
    vec3 lightDir = normalize(light.LightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);	
    vec3 diffuse = diff * light.LightColor;
	// diffuse = vec3(0.0f);
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float spec = 0.0f;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * light.LightColor;    
	// specular = vec3(0.0f);    

	float distance = length(fs_in.FragPos - light.LightPos);

    return (ambient + (diffuse + specular) * color) / (distance * distance);    
}

