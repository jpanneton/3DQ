R"(
#version 330 core

in vec2 texturePos;
out vec4 color;

uniform sampler2D imageTexture; // GL_TEXTURE0 (default)

void main()
{
    color = texture(imageTexture, texturePos);
}
)"