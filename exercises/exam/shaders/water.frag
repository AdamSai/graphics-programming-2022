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

    float offset = 1.0 / 1920;

    if (blur == true)
    {
        // get average color of neighbours
        vec4 avgColor = vec4(0.0, 0.0, 0.0, 0.0);
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                avgColor += texture(heightmapTexture, UV + vec2(i * offset, j * offset));
            }
        }

        color = avgColor;

    }
    else
    color = texture(heightmapTexture, UV);


}