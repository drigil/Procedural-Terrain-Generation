#version 330 core

in vec3 vVertex;
in vec3 vColor;
uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;

//flat keyword used for flat shading
flat out vec3 fColor;

void main() {
	gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);
	fColor = vColor; //Interpolate color
}
