// Vertex shader: Wave
// ================
#version 450 core

// DEVO INSERIRE LOCATION ???
layout (location = 0) in vec4 vPosition;
in vec4 vColor;


uniform float time; // in milliseconds

//uniform mat4 ModelViewProjectionMatrix;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling


// Lighting

layout (location = 1) in vec3 vNormal;

uniform float shininess;
//uniform vec4 vPosition, lightPosition, diffuseLight, specularLight;
//uniform mat4 ModelViewMatrix, ModelViewProjectionMatrix, NormalMatrix;

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };
uniform PointLight light;


void main() {

	vec4 v = vPosition;

	float a = 0.1;		// Ampiezza dell’oscillazione
	float omega = 0.001;	// Frequenza

	v.y = a * sin(omega*time + 10*v.x) * sin(omega*time + 10*v.z);


	//gl_Position = ModelViewProjectionMatrix * v;
	gl_Position = P * V * M * v;



	// Lighting

	vec4 diffuse, specular;

	//vec4 eyePosition = ModelViewMatrix * vPosition;
	vec4 eyePosition = V * M * vPosition;
	//vec4 eyeLightPos = lightPosition;
	vec4 eyeLightPos = V * vec4(light.position, 1.0);

	//vec3 N = normalize(NormalMatrix * Normal);
	// ...
	N = transpose(inverse(mat3(V * M))) * vNormal;
	vec3 L = normalize(vec3(eyeLightPos - eyePosition));
	vec3 E = -normalize(eyePosition.xyz);
	vec3 H = normalize(L + E);

	float diff = max(dot(L, N), 0.0);
	float spec = pow(max(dot(N, H), 0.0), shininess);
	//diffuse = diff * diffuseLight;
	diffuse = light.power * light.color * diff;// * material.diffuse;
	//specular = spec * specularLight;
	specular =  light.power * light.color * spec;// * material.specular;

	color = diffuse + specular; // OUTPUT GIUSTO ???
}

