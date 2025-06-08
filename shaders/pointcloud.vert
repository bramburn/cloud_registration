#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 mvpMatrix;
uniform float pointSize;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
    gl_PointSize = pointSize;
}
