R"(
#version 330 core

layout (location = 0) in vec2 position;

out vec2 texturePos;

void main()
{
    gl_Position = vec4(position, 0, 1.0);
    texturePos = (position + 1.0) / 2.0;
}
)"