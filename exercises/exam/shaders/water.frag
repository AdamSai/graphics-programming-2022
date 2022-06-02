#version 330 core

uniform sampler2D heightmapTexture;
uniform bool blur;
layout(location = 0) out vec3 color;
in vec2 UV;

void main()
{

    // TODO: Get average of colors around the pixel
    if (blur)
    {

        vec2 cellSize = 1.0 / vec2(720, 720);
        // Get neighbour UVs
        vec2 neighbourUVs[4];
        neighbourUVs[0] = UV + vec2(-cellSize.x, -cellSize.y);
        neighbourUVs[1] = UV + vec2(0, -cellSize.y);
        neighbourUVs[2] = UV + vec2(cellSize.x, -cellSize.y);
        neighbourUVs[3] = UV + vec2(cellSize.x, 0);

        // get neighbour colors
        vec3 neighbourColors[4];
        neighbourColors[0] = texture(heightmapTexture, neighbourUVs[0]).rgb;
        neighbourColors[1] = texture(heightmapTexture, neighbourUVs[1]).rgb;
        neighbourColors[2] = texture(heightmapTexture, neighbourUVs[2]).rgb;
        neighbourColors[3] = texture(heightmapTexture, neighbourUVs[3]).rgb;

        // get average color
        vec3 averageColor = vec3(0);
        for (int i = 0; i < 4; i++)
        {
            averageColor += neighbourColors[i];
        }
        averageColor /= 4.0;

        color = averageColor;
    }
    else
    {
        color = texture(heightmapTexture, UV).rgb;
    }
}

