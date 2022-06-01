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

    vec2 cellSize = 1.0 / vec2(720, 720);

    if (blur == true)
    {
        // Get neighbour UVs
        vec2 neighbourUVs[4 * neighbourMultiplier];
        for (int i = 0; i < neighbourMultiplier * 4; i+=4)
        {
            int multiplier = i + 1;
            neighbourUVs[i] = UV + vec2(0, cellSize.y * multiplier);
            neighbourUVs[i + 1] = UV + vec2(0, -cellSize.y * multiplier);
            neighbourUVs[i + 2] = UV + vec2(cellSize.x * multiplier, 0);
            neighbourUVs[i + 3] = UV + vec2(-cellSize.x * multiplier, 0);
        }


        // get neighbour colors
        vec4 neighbourColors[4 * neighbourMultiplier];
        for (int i = 0; i < neighbourUVs.length(); i++)
        {

            neighbourColors[i] = texture(heightmapTexture, neighbourUVs[i]);
        }


        // get average color
        vec4 averageColor = vec4(1, 1, 1, 1);
        for (int i = 0; i < neighbourColors.length(); i++)
        {
            if (neighbourColors[i].a > 0.1f)
            averageColor = mix(averageColor, neighbourColors[i], 0.5);

        }
        //    averageColor /= neighbourUVs.length();
        if (averageColor != vec4(0))
        color = averageColor;
        else
        color = texture(heightmapTexture, UV);

    }
    else
    color = texture(heightmapTexture, UV);


}