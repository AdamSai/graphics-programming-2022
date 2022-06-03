#version 330 core
layout(location = 0) out vec4 color;

uniform sampler2D canvas;
in vec2 UV;
// reference: https://en.wikipedia.org/wiki/Kernel_(image_processing)
void main()
{
    float offset = 1.0 / 1920;
    vec2 offsets[9] = vec2[](
    vec2(-offset, offset), // top-left
    vec2(0.0f, offset), // top-center
    vec2(offset, offset), // top-right
    vec2(-offset, 0.0f), // center-left
    vec2(0.0f, 0.0f), // center-center
    vec2(offset, 0.0f), // center-right
    vec2(-offset, -offset), // bottom-left
    vec2(0.0f, -offset), // bottom-center
    vec2(offset, -offset)// bottom-right
    );

    // Gausian blur
    float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
    );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(canvas, UV.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++)
    col += sampleTex[i] * kernel[i];

    color = vec4(col, 1.0);
}