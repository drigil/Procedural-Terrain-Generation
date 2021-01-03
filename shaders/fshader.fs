#version 330 core

//flat keyword used for flat shading
flat in vec3 fColor;
out vec4 outColor;
void main(void) {
    outColor = vec4(fColor, 1.0);
}
