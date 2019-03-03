R"(
#version 330 core

layout (location = 0) in vec3 pos;
out float level;

// Uniforms
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * vec4(pos.x, pos.y, pos.z, 1.0);
    level = pos.y;
}
)"