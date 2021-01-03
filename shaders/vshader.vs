#version 330 core

layout (location = 0) in vec3 vVertex;
layout (location = 1) in vec3 vColor;
layout (location = 2) in vec3 vNormal;
uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;
uniform vec3 camPosition;

vec3 lpos_world = vec3(0.0f, 2000.0f, 0.0f);
vec3 worldOrigin = vec3(0.0f, 0.0f, 0.0f);

vec3 diffLight;
float diffLightCoeff;
vec3 lightVector;

vec3 positionVector;
vec3 pos_reduced;
vec3 reflectedVector;
vec3 viewVector;

vec3 specLight;
float specLightCoeff;
float shininessCoeff;

vec3 totLight;

//flat keyword used for flat shading
flat out vec3 fColor;

void main() {
	gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);
	pos_reduced = vec3(gl_Position[0], gl_Position[1], gl_Position[2]);

	//Diffuse Light
	diffLightCoeff = 1.0f;
	lightVector = normalize(pos_reduced - lpos_world);
	diffLight = vec3(1.0f, 1.0f, 1.0f);
	diffLight = diffLightCoeff * max(dot(lightVector, vNormal), 0.0f) * diffLight;

	//Specular Light
	reflectedVector = (2 * dot(lightVector, vNormal) * vNormal) - lightVector;
	reflectedVector = normalize(reflectedVector);
	viewVector = normalize(camPosition - pos_reduced);
	specLightCoeff = 0.2f;
	shininessCoeff = 64.0f;
	specLight = vec3(1.0f, 1.0f, 1.0f);
	specLight = specLightCoeff * max(pow(dot(reflectedVector,viewVector), shininessCoeff), 0.0f) * specLight;

	totLight = diffLight + specLight;

	//Calculate fog
	float fogCoord = abs(distance(camPosition, vVertex)) / 5;
	float fogDensity = 0.04f;
	float fogIntensity = exp(-fogDensity*fogCoord);
	fogIntensity = 1.0-clamp(fogIntensity, 0.0, 1.0);
	vec3 fogColor = vec3(0.7f, 0.7f, 0.7f);

	fColor = mix(vColor * totLight, fogColor, fogIntensity); //Interpolate color

}
