#version 330 core

out vec4 FragColor;

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
	// distance (z)
    float lightDistance = length(FragPos.xyz - lightPos);
	// scale to [0, 1]
    lightDistance = lightDistance / far_plane;
    gl_FragDepth = lightDistance;
}  