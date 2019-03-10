R"(
#version 330 core

layout (location = 0) in vec2 position;

out vec2 texturePos;

uniform vec2 screenSize;
uniform float textureBounds[4];

void main()
{
    vec2 scaled = position / (0.5 * screenSize.xy);
    gl_Position = vec4(scaled - 1.0, 0, 1.0);
    texturePos = (position - vec2(textureBounds[0], textureBounds[1])) / vec2(textureBounds[2], textureBounds[3]);
}
)"