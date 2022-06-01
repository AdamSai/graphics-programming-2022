#version 330 core

uniform sampler2D heightmapTexture;
uniform sampler2D old_heightmapTexture;
uniform vec2 mousePos;
uniform float time;

in vec4 FragPos;
layout(location = 0) out vec3 color;
in vec2 UV;
const int neighbourMultiplier = 2;


void main()
{

    // TODO: Get average of colors around the pixel

    vec2 cellSize = 1.0 / vec2(720, 720);

    // Get neighbour UVs
    vec2 neighbourUVs[4 * neighbourMultiplier];
    for (int i = 0; i < neighbourMultiplier * 4; i+=4)
    {
        neighbourUVs[i] = UV + vec2(-cellSize.x * i, -cellSize.y * i);
        neighbourUVs[i + 1] = UV + vec2(0, -cellSize.y * i);
        neighbourUVs[i + 2] = UV + vec2(cellSize.x * i, -cellSize.y * i);
        neighbourUVs[i + 3] = UV + vec2(cellSize.x * i, 0);
    }


    // get neighbour colors
    vec3 neighbourColors[4 * neighbourMultiplier];
    for (int i = 0; i < neighbourUVs.length(); i++)
    {
        neighbourColors[i] = texture(heightmapTexture, neighbourUVs[i]).rgb;
    }


    // get average color
    vec3 averageColor = vec3(0);
    for (int i = 0; i < neighbourColors.length(); i++)
    {
        averageColor = mix(averageColor, neighbourColors[i], 0.5);
    }
    //    averageColor /= neighbourUVs.length();


    color = averageColor;


    //    color = texture(heightmapTexture, UV).rgb;


}

