R"(
#version 330 core

in float level;
out vec4 color;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	vec3 fragColor = hsv2rgb(vec3((1.0f - level) * 0.3f, 1.0f, level));
    color = vec4(fragColor.x, fragColor.y, fragColor.z, 1.0f);
}
)"