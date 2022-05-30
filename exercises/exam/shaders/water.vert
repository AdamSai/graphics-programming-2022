#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
out vec2 UV;
uniform float time;
uniform float waveStrength;

out vec4 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{

    //  float Lts = 1/4 * ()
    float c1 = 0.3;
    float c2 = 0.65;
    //float height = c1 * (2)
    vec3 pos = vec3(position.x, (waveStrength * sin(time + position.x)), position.z);

    float max = waveStrength;
    float min = -waveStrength / 4;
    gl_Position = projection * view *  model * vec4(pos, 1.0f);

    UV = texCoords;
    FragPos = model * vec4(pos, 1.0f);

}


// TODO: Get height of neighbours
//float Lts(float x, float y)
//{
//    return 1/4 * ()
//}
