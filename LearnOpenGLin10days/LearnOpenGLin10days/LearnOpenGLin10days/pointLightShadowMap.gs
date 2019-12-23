#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 shadowTransformMatrices[6];

out vec4 FragPos;

void main()
{
	for (int face = 0; face < 6; face++)
	{
		gl_Layer = face;
		for (int index = 0; index < 3; index++)
		{
			FragPos = gl_in[index].gl_Position;
			gl_Position = shadowTransformMatrices[face] * gl_in[index].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}