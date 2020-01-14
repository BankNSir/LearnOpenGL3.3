#version 330 core

out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPos;
uniform sampler2D gNormal;
// uniform sampler2D gAlbedoSpec; not used
uniform sampler2D noiseRotationTexture;

uniform vec3 samples[64]; // random vector of sample in TBN
uniform mat4 projection;

const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0);

void main()
{
	float radius = 0.5f; // karnel radius
	float occlusion = 0.0f;

	vec3 fragPos = texture(gPos, TexCoords).rgb;
	vec3 normal = texture(gNormal, TexCoords).rgb;
	vec3 randomRotationVector = texture(noiseRotationTexture, TexCoords * noiseScale).rgb;

	// Gram_Schmidth
	vec3 tangent = normalize(randomRotationVector - normal * dot(normal, randomRotationVector));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	
	// current depth
	//float currentDepth = texture(gPos, TexCoords).z; // viewSpace
	float bias = 0.00f;

	// karnel
	for (int i = 0; i < 64; i++)
	{
		vec3 sample = TBN * samples[i];
		vec3 offsetViewPos = fragPos + sample * radius;
		vec4 offsetClipPos = projection * vec4(offsetViewPos, 1.0f);
		vec3 offsetTexCoords = offsetClipPos.xyz / offsetClipPos.w; // perspective division
		offsetTexCoords = offsetTexCoords * 0.5 + 0.5;
		float sampleDepth = texture(gPos, offsetTexCoords.xy).z;
		
		// compare depth and adjust occlusion factor
		// sampleDepth e (-nearplane, -farplane)
		// offsetTexCoors e (0.0f, 1.0f)
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sample.z + bias < sampleDepth? 1.0f: 0.0f) * rangeCheck;
		// mostly sampleDedpth < offsetCoords.z
	}
	
	occlusion = 1.0f - occlusion/64.0f;

	FragColor = occlusion;
}