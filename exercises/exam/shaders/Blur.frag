#version 330
layout(location = 0) out vec4 color;

uniform sampler2D canvas;
in vec2 UV;
const int neighbourMultiplier = 2;

void main() {
    vec2 cellSize = 1.0 / vec2(720, 720);
    // Get neighbour UVs
    vec2 neighbourUVs[4];
    neighbourUVs[0] = UV + vec2(-cellSize.x, -cellSize.y);
    neighbourUVs[1] = UV + vec2(0, -cellSize.y);
    neighbourUVs[2] = UV + vec2(cellSize.x, -cellSize.y);
    neighbourUVs[3] = UV + vec2(cellSize.x, 0);

    // get neighbour colors
    vec4 neighbourColors[4];
    neighbourColors[0] = texture(canvas, neighbourUVs[0]);
    neighbourColors[1] = texture(canvas, neighbourUVs[1]);
    neighbourColors[2] = texture(canvas, neighbourUVs[2]);
    neighbourColors[3] = texture(canvas, neighbourUVs[3]);

    // get average color
    vec4 averageColor = texture(canvas, UV);
    for (int i = 0; i < 4; i++)
    {
        averageColor += neighbourColors[i];
    }
    averageColor /= 4.0;

    color = averageColor;
}
