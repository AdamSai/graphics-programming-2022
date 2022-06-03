#version 330 core
layout(location = 0) out vec4 color;

uniform sampler2D canvas;
in vec2 UV;

void main()
{
    color = texture(canvas, UV);
}

