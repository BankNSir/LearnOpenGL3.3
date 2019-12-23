#version 330 core

out vec4 FragColor;

in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;
uniform sampler2D parallaxDepthMap;
uniform samplerCube depthMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform float parallaxOffsetScale;

float ShadowCalculation(vec3 fragPos);
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

void main()
{   
	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec2 parallaxTexCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
	// fake fix to edge artifacts
	// if (parallaxTexCoords.x < 0.0f || parallaxTexCoords.x > 1.0f || parallaxTexCoords.y < 0.0f || parallaxTexCoords.y > 1.0f) 
	//	discard;
	vec3 color = texture(diffuseTexture, parallaxTexCoords).rgb;
    vec3 normal = texture(normalMap, parallaxTexCoords).rgb;
	normal = normalize(normal * 2 - 1);
    vec3 lightColor = vec3(1.0);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = 0 * diff * lightColor;
    // specular
    // vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPos);       
    vec3 lighting = ambient + ((1.0 - shadow) * (diffuse + specular)) * color;    
    
	// FragColor = vec4(vec3(shadow), 1.0f);

	// float currentDepth = length(fs_in.FragPos - lightPos) / far_plane;
	
    FragColor = vec4(lighting, 1.0);
	// vec3 fragToLight = fs_in.FragPos - lightPos;
	// float closestDepth = texture(depthMap, fragToLight).r;
	// FragColor = vec4(vec3(closestDepth), 1.0f);
}


float ShadowCalculation(vec3 fragPos)
{
	vec3 fragToLight = fragPos - lightPos;

	//float closestDepth = texture(depthMap, fragToLight).r;
	//closestDepth *= far_plane;
	float currentDepth = length(fragToLight);
	float shadowbias = max(0.2 * (1 - dot(normalize(fs_in.Normal), normalize(lightPos - fs_in.FragPos))), 0.05f);
	// float shadowbias = 0.3f;
	// PCF
	float shadow = 0.0f;
	float offset = 0.05f;
	int samples = 6;
	for (float x = -offset; x < offset; x += 2 * offset / samples)
	{
		for (float y = -offset; y < offset; y += 2 * offset / samples)
		{
			for (float z = -offset; z < offset; z += 2 * offset / samples)
			{
				float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r;
				float InShadow = (currentDepth - shadowbias) > closestDepth * far_plane? 1.0: 0.0;
				shadow += InShadow;
			}
		}
	}

	shadow /= (samples * samples * samples);
	return shadow;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height =  texture(parallaxDepthMap, texCoords).r;    
    // vec2 p = viewDir.xy / viewDir.z * (height * parallaxOffsetScale);
	vec2 p = -normalize(viewDir).xy * 0.01f;
	vec2 currentTexCoord = texCoords;
	float currentLayerDepth = 0.0f;
	float currentDepthValue = texture(parallaxDepthMap, currentTexCoord).r;
	float layerDepth = 0.1f;
	// int minLayers = 8;
	// int maxLayers = 32;
	// float nLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0f, 0.0f, 1.0f), viewDir)));
	// float layerDepth = 1.0f / nLayers; 
	// portion of p 

	while (currentLayerDepth < currentDepthValue)
	{
		currentTexCoord += layerDepth * p;
		currentDepthValue = texture(parallaxDepthMap, currentTexCoord).r;
		currentLayerDepth += layerDepth;
	}

	float prevDepthValue = texture(parallaxDepthMap, currentTexCoord - layerDepth * p).r;
	float afterDepth = currentLayerDepth - currentDepthValue;
	float beforeDepth = prevDepthValue - (currentLayerDepth - layerDepth);
	float weight = afterDepth / (afterDepth + beforeDepth);
	vec2 prevTexCoord = currentTexCoord - layerDepth * p;
    return currentTexCoord * weight + prevTexCoord * (1 - weight);    
} 