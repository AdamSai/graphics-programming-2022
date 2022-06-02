#version 330 core

uniform sampler2D heightmapTexture;
uniform sampler2D old_heightmapTexture;
uniform vec2 mousePos;
uniform float time;
uniform bool blur;

in vec4 FragPos;
layout(location = 0) out vec4 color;
in vec2 UV;
const int neighbourMultiplier = 2;


void main()
{

    // TODO: Get average of colors around the pixel


    if (blur == true)
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
            sampleTex[i] = vec3(texture(heightmapTexture, UV.st + offsets[i]));
        }
        vec3 col = vec3(0.0);
        for (int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

        color = vec4(col, 1.0);

    }
    else
    color = texture(heightmapTexture, UV);


}