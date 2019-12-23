#version 330 core

out vec4 FragColor;

in VS_OUT
{
	vec2 TexCoords;
} fs_in;

struct Light
{
	vec3 LightPos;
	vec3 LightColor;
	float Radius;
};

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D albedoSpecTexture;
uniform vec3 viewPos;
uniform Light lights[4];
uniform float exposure;

vec3 PhongCalculation(Light light);

vec3 POSITION = texture(positionTexture, fs_in.TexCoords).rgb;
vec3 NORMAL = texture(normalTexture, fs_in.TexCoords).rgb;
vec3 COLOR = texture(albedoSpecTexture, fs_in.TexCoords).rgb;
float SPECULAR = texture(albedoSpecTexture, fs_in.TexCoords).a;

void main()
{            
	// for each light in light array
	vec3 result = vec3(0.0f);

	for (int i = 0; i < 4; i++)
	{
		if (length(POSITION - lights[i].LightPos) < lights[i].Radius)
			result += PhongCalculation(lights[i]);    	
	}
	//reinheart hdr
	//result = vec3(1.0f) - 1 / (result + vec3(1.0f));

	// hdr
	result = vec3(1.0f) -exp(-result * exposure);
    FragColor = vec4(result, 1.0f);
}

vec3 PhongCalculation(Light light)
{
	vec3 color = COLOR;
	// vec3 color = vec3(1.0f);
    vec3 normal = NORMAL;
    // ambient
    vec3 ambient = 0.0f * COLOR;
    // diffuse
    vec3 lightDir = normalize(light.LightPos - POSITION);
    float diff = max(dot(lightDir, normal), 0.0);	
    vec3 diffuse = diff * light.LightColor;
	// diffuse = vec3(0.0f);
    // specular
    vec3 viewDir = normalize(viewPos - POSITION);
    float spec = 0.0f;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * light.LightColor;    
	// specular = vec3(0.0f);    

	float distance = length(POSITION - light.LightPos);

    return (ambient + (diffuse + specular) * color) / (distance * distance);    
}

