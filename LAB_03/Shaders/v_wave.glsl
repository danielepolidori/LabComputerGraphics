// Vertex shader: Wave
// ================
#version 450 core

// DEVO INSERIRE LOCATION ???
in vec4 vPosition;
in vec4 vColor;


uniform float time; // in milliseconds

//uniform mat4 ModelViewProjectionMatrix;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling


void main() {

	vec4 v = vPosition;

	float a = 0.1;		// Ampiezza dell’oscillazione
	float omega = 0.001;	// Frequenza

	v.y = a * sin(omega*time + 10*v.x) * sin(omega*time + 10*v.z);


	//gl_Position = ModelViewProjectionMatrix * v;
	gl_Position = P * V * M * v;



	// Lighting
	
}

