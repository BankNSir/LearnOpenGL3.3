#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in vec3 vColor[];

out vec3 fColor;

void build_house(vec4 position)
{
	fColor = vColor[0];
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(0.2, -0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(0.2, 0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(0.0, 0.4, 0.0, 0.0);
	fColor = vec3(1.0f, 1.0f, 1.0f);
	EmitVertex();
	EndPrimitive();
}

void main()
{
	build_house(gl_in[0].gl_Position);
}