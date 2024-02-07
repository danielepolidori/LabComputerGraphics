// Vertex shader: Toon shading
// ================
#version 450 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

out vec3 N;
out vec3 L;
out vec3 E;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };
uniform PointLight light;


void main()
{
	gl_Position = P * V * M * vec4(vPosition, 1.0);

	vec4 eyePosition = V * M * vec4(vPosition, 1.0);
	vec4 eyeLightPos = V * vec4(light.position, 1.0);

	E = -eyePosition.xyz;
	L = (eyeLightPos - eyePosition).xyz;
	N = transpose(inverse(mat3(V * M))) * vNormal;
} 

