// Vertex shader: Wave
// ================
#version 450 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;   /// Lighting


uniform float time; // in milliseconds

uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling


// Lighting

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };
uniform PointLight light;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
uniform Material material;


out vec3 Color;   /// resulting color from lighting calculations



void main() {

	vec3 v = vPosition;

	float a = 0.1;		// Ampiezza dell’oscillazione
	float omega = 0.001;	// Frequenza

	v.y = a * sin(omega*time + 10*v.x) * sin(omega*time + 10*v.z);

	gl_Position = P * V * M * vec4(v, 1.0);


	// Lighting

	vec3 ambient = light.power * material.ambient;

	vec3 diffuse, specular;

	vec4 eyePosition = V * M * vec4(vPosition, 1.0);
	//vec4 eyeLightPos = V * vec4(light.position, 1.0);
	vec4 eyeLightPos = vec4(light.position, 1.0);   // La luce arriva da un punto diverso dal punto luce (evito la moltiplicazione per V)

	vec3 N = normalize(transpose(inverse(mat3(V * M))) * vNormal);
	vec3 L = normalize(vec3(eyeLightPos - eyePosition));
	vec3 E = -normalize(eyePosition.xyz);
	vec3 H = normalize(L + E);

	float diff = max(dot(L, N), 0.0);
	float spec = pow(max(dot(N, H), 0.0), material.shininess);
	//diffuse = light.power * light.color * diff * material.diffuse;
	diffuse = diff * material.diffuse;
	//specular =  light.power * light.color * spec * material.specular;
	specular = spec * material.specular;

	//Color = diffuse + specular;
	Color = ambient + diffuse + specular;
}

