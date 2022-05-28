#version 330 core
layout (location = 0) in vec3 position;
uniform float time;
uniform float waveStrength;

out vec4 FragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{

    vec3 pos = vec3(position.x, (waveStrength * sin(time + position.x)), position.z);

    float max = waveStrength;
    float min = -waveStrength / 4;
    gl_Position = projection * view *  model * vec4(pos, 1.0f);
    float distanceY = (pos.y - min) / (max - min);
    FragColor  = vec4(distanceY, 1, 1, 1);
}