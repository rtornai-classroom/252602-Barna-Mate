#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 matModelView;
uniform mat4 matProjection;

void main()
{
    gl_Position = matProjection * matModelView * vec4(aPos, 1.0);
}