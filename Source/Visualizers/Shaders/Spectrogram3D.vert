R"(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 uv;

out vec3 fragColor;

// Uniforms
uniform sampler2D imageTexture; // GL_TEXTURE0 (default)
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
	vec4 texelValue = texture(imageTexture, uv.xy);
	// Alpha channel is used for height
    gl_Position = projectionMatrix * viewMatrix * vec4(position.x, uv.z * texelValue.a, position.z, 1.0);
    fragColor = texelValue.rgb;
}
)"