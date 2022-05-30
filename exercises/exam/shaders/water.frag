#version 330 core

in vec4 FragPos;
void main()
{
    if (FragPos.y >= 3)
    gl_FragColor  = vec4(1, 1, 1, 1);

    else if (FragPos.y >= 1)
    gl_FragColor  = vec4(0.38, 0.41, 1, 1);

    else if (FragPos.y >= 0.2)
    gl_FragColor  = vec4(0.65, 0.66, 1, 1);

    else
    gl_FragColor  = vec4(0, 0.05, 1, 1);
}