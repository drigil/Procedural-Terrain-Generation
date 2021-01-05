#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


in vec3 vVertex;
in vec3 vColor;
uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;


void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    //gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);
    //fColor = vColor; //Interpolate color
}