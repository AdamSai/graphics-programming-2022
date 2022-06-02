#version 330 core

uniform sampler2D heightmapTexture;
uniform bool blur;
layout(location = 0) out vec3 color;
in vec2 UV;

void main()
{
    color = texture(heightmapTexture, UV).rgb;
}

